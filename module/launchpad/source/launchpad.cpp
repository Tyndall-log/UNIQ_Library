// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "launchpad.h"

using namespace std;
using namespace juce;

namespace uniq
{
	void launchpad::midi_callback::handleIncomingMidiMessage(MidiInput* /*source*/, const MidiMessage& message)
	{
		//printHex(message.getRawData(), message.getRawDataSize());
		if (callback_function) callback_function((uint8_t*)message.getRawData(), message.getRawDataSize());
	}
	
	void launchpad::midi_callback::printHex(const uint8_t* data, size_t length)
	{
		log::println("MIDI_IN: " + String::toHexString(data, static_cast<int>(length)).toStdString());
	}
	
	void launchpad::midi_callback::callback_set(function<void(uint8_t*, int)>&& callback)
	{
		callback_function = callback;
	}
	
	void launchpad::init()
	{
	
	}
	
	std::string launchpad::launchpad_kind_name_get(juce::MidiDeviceInfo& mdi)
	{
		auto& identifier = mdi.identifier;
		
		if (!identifier.startsWith(R"(\\?\usb#)")) return "";
		
		int pos;
		
		pos = identifier.indexOf(8, "vid_");
		const auto& vid = 0 <= pos ? identifier.substring(pos + 4, pos + 8).toStdString() : "0000";
		
		pos = identifier.indexOf(pos + 9, "pid_");
		const auto& pid = 0 <= pos ? identifier.substring(pos + 4, pos + 8).toStdString() : "0000";
		
		//허용하는 런치패드인지 확인
		auto it = VPID_map.find(vid + pid);
		if (it == VPID_map.end()) return ""; //지원되는 런치패드 아님.
		
		pos = identifier.indexOf(pos + 9, "\\global");
		auto global_num = 0 <= pos ? identifier.substring(pos + 1).toStdString() : "0";
		auto num = get<1>(it->second);
		auto global = 0 < num ? "global-" + String(++num) : "global";
		
		if (global_num != global) return ""; //중복 건너뛰기
		return get<0>(it->second);
	}
	
	launchpad::midi_device_info::midi_device_info(const juce::MidiDeviceInfo&& info) : MidiDeviceInfo(info){}
	
	launchpad::midi_device_info::midi_device_info(const juce::MidiDeviceInfo&& info, const juce::String& name) : MidiDeviceInfo(info)
	{
		this->kind_name = name.toStdString();
	}
	
	launchpad::launchpad(shared_ptr<AudioDeviceManager>& adm)
	{
		deviceManager = adm;
		input_callback = make_unique<midi_callback>();
		{
			SpinLock::ScopedLockType lock(mutex);
			if (launchpad_list.empty())
			{
				LED_timer = make_unique<LED_global_timer>();
				LED_timer->startTimer(4);
			}
			launchpad_list.insert(this);
			LED_grid_current = vector<vector<VRGB>>(LED_w, vector<VRGB>(LED_h));
			LED_grid_target = vector<vector<VRGB>>(LED_w, vector<VRGB>(LED_h));
			LED_raw_data = make_unique<uint8[]>(static_cast<size_t>(LED_w) * LED_h * 5 + 6);
			copy_n("00'20'29'02'0D'03"_hex, 6, LED_raw_data.get()); //기본 명령어 헤더
			automatic_transmission = true;
			immediate_transmission = false;
		}
	}
	
	launchpad::launchpad(shared_ptr<AudioDeviceManager>& adm, const midi_device_info& mdi_input, const midi_device_info& mdi_output)
	: launchpad(adm)
	{
		midi_input_set(mdi_input);
		midi_output_set(mdi_output);
	}
	
	launchpad::~launchpad()
	{
		{
			SpinLock::ScopedLockType lock(mutex);
			launchpad_list.erase(this);
			if (launchpad_list.empty())
			{
				LED_timer->stopTimer();
				LED_timer.reset();
			}
		}
		deviceManager.reset();
	}
	
	bool launchpad::midi_input_set(const midi_device_info& mdi_input)
	{
		if (input)
		{
			input->stop();
			input.reset();
		}
		input = MidiInput::openDevice(mdi_input.identifier, input_callback.get());
		if (!input)
		{
			log::println("input is null");
			return false;
		}
		midi_input_kind_name = mdi_input.kind_name;
		input->start();
		return true;
		
	}
	
	bool launchpad::midi_output_set(const midi_device_info& mdi_output)
	{
		if (output)
		{
			output.reset();
		}
		output = MidiOutput::openDevice(mdi_output.identifier);
		if (!output)
		{
			log::println("output is null");
			return false;
		}
		midi_output_kind_name = mdi_output.kind_name;
		return true;
	}
	
	void launchpad::message_send_now(juce::MidiMessage& message)
	{
		if (!output)
		{
			log::println("output is null");
			return;
		}
		output->sendMessageNow(message);
	}
	
	void launchpad::hex_send(const juce::String& hex)
	{
		if (!output) return;
		auto k = hexStringToBytes(hex);
		auto m = MidiMessage::createSysExMessage(&k[0], static_cast<int>(k.size()));
		message_send_now(m);
	}
	
	void launchpad::hex_send(const juce::uint8* hex, size_t length)
	{
		if (!output) return;
		auto m = MidiMessage::createSysExMessage(hex, static_cast<int>(length));
		message_send_now(m);
	}
	
	void launchpad::LED_send()
	{
		if (!output) return;
		auto p = LED_raw_data.get() + 6;
		for (auto x = 0; x < LED_w; x++)
		{
			auto& c_x = LED_grid_current[x];
			auto& t_x = LED_grid_target[x];
			for (auto y = 0; y < LED_h; y++)
			{
				auto& c_xy = c_x[y];
				auto& t_xy = t_x[y];
				if (c_xy == t_xy) continue;
				c_xy = t_xy;
				if (t_xy.v == 0xFF)
				{
					*p++ = 0x03;
					*p++ = static_cast<uint8>(y * 10 + x);
					*p++ = t_xy.r;
					*p++ = t_xy.g;
					*p++ = t_xy.b;
				}
				else
				{
					*p++ = 0x00;
					*p++ = static_cast<uint8>(y * 10 + x);
					*p++ = t_xy.v;
				}
			}
		}
		if (p - LED_raw_data.get() <= 6) return;
		//printHex(p - 5, 5);
		auto m = MidiMessage::createSysExMessage(LED_raw_data.get(), static_cast<int>(p - LED_raw_data.get()));
		output->sendMessageNow(m);
	}
	
	void launchpad::rgb_set(uint8 x, uint8 y, uint8 r, uint8 g, uint8 b)
	{
		if (LED_w < x || LED_h < y) throw range_error("x 또는 y 범위 오류");
		SpinLock::ScopedLockType lock(mutex);
		LED_grid_target[x][y] = VRGB{ 0xFF,static_cast<uint8>(r >> 1),static_cast<uint8>(g >> 1),static_cast<uint8>(b >> 1) };
		if (immediate_transmission)
		{
			auto p = LED_raw_data.get() + 6;
			*p++ = 0x03;
			*p++ = static_cast<uint8>(y * 10 + x);
			*p++ = r;
			*p++ = g;
			*p++ = b;
			auto m = MidiMessage::createSysExMessage(LED_raw_data.get(), static_cast<int>(p - LED_raw_data.get()));
			output->sendMessageNow(m);
			LED_grid_current[x][y] = VRGB{ 0xFF,static_cast<uint8>(r >> 1),static_cast<uint8>(g >> 1),static_cast<uint8>(b >> 1) };
		}
	}
	
	void launchpad::velocity_set(uint8 x, uint8 y, uint8 v)
	{
		if (LED_w < x || LED_h < y) throw range_error("x 또는 y 범위 오류");
		SpinLock::ScopedLockType lock(mutex);
		LED_grid_target[x][y] = VRGB{ v,0,0,0 };
		if (immediate_transmission)
		{
			auto p = LED_raw_data.get() + 6;
			*p++ = 0x00;
			*p++ = static_cast<uint8>(y * 10 + x);
			*p++ = v;
			auto m = MidiMessage::createSysExMessage(LED_raw_data.get(), static_cast<int>(p - LED_raw_data.get()));
			output->sendMessageNow(m);
			LED_grid_current[x][y] = VRGB{ v,0,0,0 };
		}
	}
	
	void launchpad::program_mode_set(bool flag)
	{
		auto t = "00'20'29'02'0D'0E'01"_hex;
		auto f = "00'20'29'02'0D'0E'00"_hex;
		MidiMessage 메시지 = MidiMessage::createSysExMessage(flag ? t : f, 7);
		message_send_now(메시지);
	}
	
	void launchpad::automatic_transmission_set(bool flag)
	{
		if (flag == automatic_transmission) return;
		SpinLock::ScopedLockType lock(mutex);
		automatic_transmission = flag;
	}
	
	void launchpad::immediate_transmission_set(bool flag)
	{
		if (flag == immediate_transmission) return;
		SpinLock::ScopedLockType lock(mutex);
		immediate_transmission = flag;
	}
	
	void launchpad::immediate_transmission_global_timer_set(int ms)
	{
		LED_timer->startTimer(ms);
	}
	
	void launchpad::input_callback_set(std::function<void(std::uint8_t*, int)>&& callback)
	{
		input_callback->callback_set(std::move(callback));
	}
	
	vector<launchpad::midi_device_info> launchpad::get_available_input_list()
	{
		vector<launchpad::midi_device_info> devices;
		auto id_list = map<string, uint8_t>();
		auto availableDevices = MidiInput::getAvailableDevices();
		
		for (auto& deviceInfo : availableDevices)
		{
			auto name = launchpad_kind_name_get(deviceInfo);
			if (name.empty()) continue;
			devices.emplace_back(std::move(deviceInfo), name);
		}
		
		return devices;
	}
	
	vector<launchpad::midi_device_info> launchpad::get_available_output_list()
	{
		vector<launchpad::midi_device_info> devices;
		auto id_list = map<string, uint8_t>();
		auto availableDevices = MidiOutput::getAvailableDevices();
		
		for (auto& deviceInfo : availableDevices)
		{
			auto name = launchpad_kind_name_get(deviceInfo);
			if (name.empty()) continue;
			devices.emplace_back(std::move(deviceInfo), name);
		}
		
		return devices;
	}
	
	string launchpad::input_identifier_get()
	{
		return input->getIdentifier().toStdString();
	}
	
	string launchpad::output_identifier_get()
	{
		return output->getIdentifier().toStdString();
	}
	
	vector<uint8> hexStringToBytes(const String& input)
	{
		vector<uint8> result;
		for (int i = 0; i < input.length(); i++)
		{
			if (isxdigit(input[i]))
			{
				// 유효한 16진수 문자인지 체크
				// 두 문자씩 읽어서 uint8_t 타입으로 변환
				uint8 byte = (uint8)stoi(input.substring(i, i + 2).toStdString(), nullptr, 16);
				result.push_back(byte);
				i++; // 한 바이트씩 읽기 위해 인덱스를 1 증가
			}
		}
		return result;
	}
	
	map<string, tuple<string, uint8>> const launchpad::VPID_map = {
			//1235 -> Focusrite-Novation
			{"1235" "000E", {"Novation Launchpad", 0_uc}},
			{"1235" "0020", {"Novation Launchpad S", 0_uc}},
			{"1235" "0036", {"Novation Launchpad Mini", 0_uc}},
			{"1235" "0051", {"Novation Launchpad Pro", 0_uc}},
			{"1235" "0069", {"Novation Launchpad MK2", 0_uc}},
			{"1235" "006A", {"Novation Launchpad MK2 2", 0_uc}},
			{"1235" "006B", {"Novation Launchpad MK2 3", 0_uc}},
			{"1235" "006C", {"Novation Launchpad MK2 4", 0_uc}},
			{"1235" "006D", {"Novation Launchpad MK2 5", 0_uc}},
			{"1235" "006E", {"Novation Launchpad MK2 6", 0_uc}},
			{"1235" "006F", {"Novation Launchpad MK2 7", 0_uc}},
			{"1235" "0070", {"Novation Launchpad MK2 8", 0_uc}},
			{"1235" "0071", {"Novation Launchpad MK2 9", 0_uc}},
			{"1235" "0072", {"Novation Launchpad MK2 10", 0_uc}},
			{"1235" "0073", {"Novation Launchpad MK2 11", 0_uc}},
			{"1235" "0074", {"Novation Launchpad MK2 12", 0_uc}},
			{"1235" "0075", {"Novation Launchpad MK2 13", 0_uc}},
			{"1235" "0076", {"Novation Launchpad MK2 14", 0_uc}},
			{"1235" "0077", {"Novation Launchpad MK2 15", 0_uc}},
			{"1235" "0078", {"Novation Launchpad MK2 16", 0_uc}},
			{"1235" "0103", {"Novation Launchpad X", 1_uc}},
			{"1235" "0113", {"Novation Launchpad Mini MK3", 1_uc}},
			{"1235" "0123", {"Novation Launchpad Pro MK3", 1_uc}}
	};
	
	unique_ptr<launchpad::LED_global_timer> launchpad::LED_timer = nullptr;
	set<launchpad*> launchpad::launchpad_list = set<launchpad*>();
	SpinLock launchpad::mutex = SpinLock();
	
	void launchpad::LED_global_timer::hiResTimerCallback()
	{
		SpinLock::ScopedLockType lock(mutex);
		for (auto& l : launchpad_list)
		{
			if (!l->automatic_transmission) continue;
			l->LED_send();
		}
		//printf("!");
	}
}