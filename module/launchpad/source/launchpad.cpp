// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "launchpad.h"

#include <utility>

using namespace std;
using namespace juce;

namespace uniq
{
	void launchpad::midi_callback::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message)
	{
		//printHex(message.getRawData(), message.getRawDataSize());
		if (callback_function) callback_function(message.getRawData(), message.getRawDataSize());
		// for (auto& &[k, f] : callback_function_map_)
		// {
		// 	f(const_cast<uint8_t*>(message.getRawData()), message.getRawDataSize());
		// }
	}
	
	void launchpad::midi_callback::printHex(const uint8_t* data, size_t length)
	{
		log::println("MIDI_IN: " + String::toHexString(data, static_cast<int>(length)).toStdString());
	}

	void launchpad::midi_callback::callback_set(function<void(const uint8_t*, int)> callback)
	{
		callback_function = std::move(callback);
	}

	// int launchpad::midi_callback::callback_add(std::function<void(std::uint8_t *, int)> &&callback)
	// {
	// 	callback_function_map_[++callback_function_map_index] = std::move(callback);
	// 	return callback_function_map_index;
	// }
	//
	// bool launchpad::midi_callback::callback_remove(const int callback_id)
	// {
	// 	return callback_function_map_.erase(callback_id) != 0;
	// }

	void launchpad::init()
	{
	
	}
	
	std::string launchpad::launchpad_kind_name_get(juce::MidiDeviceInfo& mdi)
	{
		auto& identifier = mdi.identifier;
		
		if (!identifier.startsWith(R"(\\?\usb#)")) return "";

		int pos = identifier.indexOf(8, "vid_");
		const auto& vid = 0 <= pos ? identifier.substring(pos + 4, pos + 8).toStdString() : "0000";
		
		pos = identifier.indexOf(pos + 9, "pid_");
		const auto& pid = 0 <= pos ? identifier.substring(pos + 4, pos + 8).toStdString() : "0000";

		//노베이션 vid인지 확인
		if (vid != "1235") return "";

		//허용하는 런치패드인지 확인
		const auto it = VPID_map.find(vid + pid);
		if (it == VPID_map.end())
		{
			log::warn("지원되는 런치패드가 아닙니다. VID: " + vid + ", PID: " + pid);
			return ""; //지원되는 런치패드 아님.
		}

		pos = identifier.indexOf(pos + 9, "\\global");
		const auto global_num = 0 <= pos ? identifier.substring(pos + 1).toStdString() : "0";
		auto num = get<1>(it->second);
		const auto global = 0 < num ? "global-" + String(++num) : "global";
		
		if (global_num != global) return ""; //중복 건너뛰기
		return get<0>(it->second);
	}

	void launchpad::input_button_callback(const uint8_t* const data, const int size)
	{
		if (size < 3) return;
		auto& note = data[0];
		const auto x = data[1] % 10;
		const auto y = data[1] / 10;
		const auto v = data[2];
		SpinLock::ScopedLockType lock(mutex);
		for (auto& [k, f] : input_callback_function_map_)
		{
			f(data, size);
		}
		if (0x90 <= note && note <= 0x9F || 0xB0 <= note && note <= 0xBF)
		{
			if (0 < v)
			{
				for (auto& [k, f] : input_button_down_callback_map_)
					f(x, y, v);
			}
			else
			{
				for (auto& [k, f] : input_button_up_callback_map_)
					f(x, y);
			}
		}
		else if (0x80 <= note && note <= 0x8F)
		{
			for (auto& [k, f] : input_button_up_callback_map_)
				f(x, y);
		}
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
		// input_callback->callback_set(input_button_callback);
		input_callback->callback_set([this](const uint8_t* data, const int size)
		{
			input_button_callback(data, size);
		});
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

	launchpad::launchpad(const shared_ptr<audio_device_manager> &adm, const midi_device_info* mdi_input, const midi_device_info* mdi_output)
	: launchpad(adm->get())
	{
		if (mdi_input) midi_input_set(*mdi_input);
		if (mdi_output) midi_output_set(*mdi_output);
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
			log::error("input is null");
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
			log::error("output is null");
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
	
	void launchpad::rgb_set(const uint8 x, const uint8 y, const uint8 r, const uint8 g, const uint8 b)
	{
		if (LED_w < x || LED_h < y)
		{
			log::error("x 또는 y 범위 오류");
			return;
		}
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
	
	void launchpad::velocity_set(const uint8 x, const uint8 y, const uint8 v)
	{
		if (LED_w < x || LED_h < y)
		{
			log::error("x 또는 y 범위 오류");
			return;
		}
		SpinLock::ScopedLockType lock(mutex);
		LED_grid_target[x][y] = VRGB{ v, 0, 0, 0 };
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
	
	void launchpad::program_mode_set(const bool flag)
	{
		auto t = "00'20'29'02'0D'0E'01"_hex;
		auto f = "00'20'29'02'0D'0E'00"_hex;
		MidiMessage 메시지 = MidiMessage::createSysExMessage(flag ? t : f, 7);
		message_send_now(메시지);
	}
	
	void launchpad::automatic_transmission_set(const bool flag)
	{
		if (flag == automatic_transmission) return;
		SpinLock::ScopedLockType lock(mutex);
		automatic_transmission = flag;
	}
	
	void launchpad::immediate_transmission_set(const bool flag)
	{
		if (flag == immediate_transmission) return;
		SpinLock::ScopedLockType lock(mutex);
		immediate_transmission = flag;
	}
	
	void launchpad::immediate_transmission_global_timer_set(const int ms)
	{
		LED_timer->startTimer(ms);
	}

	int launchpad::input_callback_add(std::function<void(const std::uint8_t *, int)> &&callback)
	{
		// input_callback->callback_set(std::move(callback));
		input_callback_function_map_[++input_callback_function_map_index] = std::move(callback);
		return input_callback_function_map_index;
	}

	bool launchpad::input_callback_remove(const int callback_id)
	{
		return input_callback_function_map_.erase(callback_id) != 0;
	}

	auto launchpad::input_button_down_callback_add(std::function<void(std::uint8_t, std::uint8_t, std::uint8_t)> &&callback) -> int
	{
		input_button_down_callback_map_[++input_callback_function_map_index] = std::move(callback);
		return input_callback_function_map_index;
	}

	bool launchpad::input_button_down_callback_remove(int callback_id)
	{
		return input_button_down_callback_map_.erase(callback_id) != 0;
	}

	auto launchpad::input_button_up_callback_add(std::function<void(std::uint8_t, std::uint8_t)> &&callback) -> int
	{
		input_button_up_callback_map_[++input_callback_function_map_index] = std::move(callback);
		return input_callback_function_map_index;
	}

	bool launchpad::input_button_up_callback_remove(int callback_id)
	{
		return input_button_up_callback_map_.erase(callback_id) != 0;
	}

	vector<launchpad::midi_device_info> launchpad::get_available_input_list()
	{
		vector<midi_device_info> devices;
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
		vector<midi_device_info> devices;
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
		{"1235" "000e", {"Novation Launchpad", 0_uc}},
		{"1235" "0020", {"Novation Launchpad S", 0_uc}},
		{"1235" "0036", {"Novation Launchpad Mini", 0_uc}},
		{"1235" "0051", {"Novation Launchpad Pro", 0_uc}},
		{"1235" "0069", {"Novation Launchpad MK2 1", 0_uc}},
		{"1235" "006a", {"Novation Launchpad MK2 2", 0_uc}},
		{"1235" "006b", {"Novation Launchpad MK2 3", 0_uc}},
		{"1235" "006v", {"Novation Launchpad MK2 4", 0_uc}},
		{"1235" "006d", {"Novation Launchpad MK2 5", 0_uc}},
		{"1235" "006e", {"Novation Launchpad MK2 6", 0_uc}},
		{"1235" "006f", {"Novation Launchpad MK2 7", 0_uc}},
		{"1235" "0070", {"Novation Launchpad MK2 8", 0_uc}},
		{"1235" "0071", {"Novation Launchpad MK2 9", 0_uc}},
		{"1235" "0072", {"Novation Launchpad MK2 10", 0_uc}},
		{"1235" "0073", {"Novation Launchpad MK2 11", 0_uc}},
		{"1235" "0074", {"Novation Launchpad MK2 12", 0_uc}},
		{"1235" "0075", {"Novation Launchpad MK2 13", 0_uc}},
		{"1235" "0076", {"Novation Launchpad MK2 14", 0_uc}},
		{"1235" "0077", {"Novation Launchpad MK2 15", 0_uc}},
		{"1235" "0078", {"Novation Launchpad MK2 16", 0_uc}},
		{"1235" "0103", {"Novation Launchpad X 1", 1_uc}},
		{"1235" "0104", {"Novation Launchpad X 2", 1_uc}},
		{"1235" "0105", {"Novation Launchpad X 3", 1_uc}},
		{"1235" "0106", {"Novation Launchpad X 4", 1_uc}},
		{"1235" "0107", {"Novation Launchpad X 5", 1_uc}},
		{"1235" "0108", {"Novation Launchpad X 6", 1_uc}},
		{"1235" "0109", {"Novation Launchpad X 7", 1_uc}},
		{"1235" "010a", {"Novation Launchpad X 8", 1_uc}},
		{"1235" "010b", {"Novation Launchpad X 9", 1_uc}},
		{"1235" "010c", {"Novation Launchpad X 10", 1_uc}},
		{"1235" "010d", {"Novation Launchpad X 11", 1_uc}},
		{"1235" "010e", {"Novation Launchpad X 12", 1_uc}},
		{"1235" "010f", {"Novation Launchpad X 13", 1_uc}},
		{"1235" "0110", {"Novation Launchpad X 14", 1_uc}},
		{"1235" "0111", {"Novation Launchpad X 15", 1_uc}},
		{"1235" "0112", {"Novation Launchpad X 16", 1_uc}},
		{"1235" "0113", {"Novation Launchpad Mini MK3 1", 1_uc}},
		{"1235" "0114", {"Novation Launchpad Mini MK3 2", 1_uc}},
		{"1235" "0115", {"Novation Launchpad Mini MK3 3", 1_uc}},
		{"1235" "0116", {"Novation Launchpad Mini MK3 4", 1_uc}},
		{"1235" "0117", {"Novation Launchpad Mini MK3 5", 1_uc}},
		{"1235" "0118", {"Novation Launchpad Mini MK3 6", 1_uc}},
		{"1235" "0119", {"Novation Launchpad Mini MK3 7", 1_uc}},
		{"1235" "011a", {"Novation Launchpad Mini MK3 8", 1_uc}},
		{"1235" "011b", {"Novation Launchpad Mini MK3 9", 1_uc}},
		{"1235" "011c", {"Novation Launchpad Mini MK3 10", 1_uc}},
		{"1235" "011d", {"Novation Launchpad Mini MK3 11", 1_uc}},
		{"1235" "011e", {"Novation Launchpad Mini MK3 12", 1_uc}},
		{"1235" "011f", {"Novation Launchpad Mini MK3 13", 1_uc}},
		{"1235" "0120", {"Novation Launchpad Mini MK3 14", 1_uc}},
		{"1235" "0121", {"Novation Launchpad Mini MK3 15", 1_uc}},
		{"1235" "0122", {"Novation Launchpad Mini MK3 16", 1_uc}},
		{"1235" "0123", {"Novation Launchpad Pro MK3 1", 1_uc}},
		{"1235" "0124", {"Novation Launchpad Pro MK3 2", 1_uc}},
		{"1235" "0125", {"Novation Launchpad Pro MK3 3", 1_uc}},
		{"1235" "0126", {"Novation Launchpad Pro MK3 4", 1_uc}},
		{"1235" "0127", {"Novation Launchpad Pro MK3 5", 1_uc}},
		{"1235" "0128", {"Novation Launchpad Pro MK3 6", 1_uc}},
		{"1235" "0129", {"Novation Launchpad Pro MK3 7", 1_uc}},
		{"1235" "012a", {"Novation Launchpad Pro MK3 8", 1_uc}},
		{"1235" "012b", {"Novation Launchpad Pro MK3 9", 1_uc}},
		{"1235" "012c", {"Novation Launchpad Pro MK3 10", 1_uc}},
		{"1235" "012d", {"Novation Launchpad Pro MK3 11", 1_uc}},
		{"1235" "012e", {"Novation Launchpad Pro MK3 12", 1_uc}},
		{"1235" "012f", {"Novation Launchpad Pro MK3 13", 1_uc}},
		{"1235" "0130", {"Novation Launchpad Pro MK3 14", 1_uc}},
		{"1235" "0131", {"Novation Launchpad Pro MK3 15", 1_uc}},
		{"1235" "0132", {"Novation Launchpad Pro MK3 16", 1_uc}},
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