// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "launchpad.h"

using namespace std;
using namespace juce;

namespace uniq
{
	class MyClass
	{
	public:
		MyClass();
		~MyClass();

	private:

	};

	MyClass::MyClass()
	{
		std::cout << endl;
	}

	MyClass::~MyClass()
	{
	}

	void launchpad::midicallback::handleIncomingMidiMessage(MidiInput* /*source*/, const MidiMessage& message)
	{
		//cout << k << endl;
		//throw std::runtime_error("테스트");
		printHex(message.getRawData(), message.getRawDataSize());
	}

	void launchpad::midicallback::printHex(const uint8_t* data, size_t length)
	{
		for (size_t i = 0; i < length; i++)
		{
			printf("%02x ", data[i]);
		}
		printf("\n");
	}

	void launchpad::init()
	{

	}

	launchpad::launchpad(shared_ptr<AudioDeviceManager> adm, MidiDeviceInfo mdi)
	{
		deviceManager = adm;
		input_callback = make_unique<midicallback>();
		input = MidiInput::openDevice(mdi.identifier, input_callback.get());
		output = MidiOutput::openDevice(mdi.identifier);
		if (!deviceManager)
		{
			deviceManager = std::make_shared<AudioDeviceManager>();
		}
		if (!deviceManager->isMidiInputDeviceEnabled(mdi.identifier))
			deviceManager->setMidiInputDeviceEnabled(mdi.identifier, true);
		kind_name = mdi.name.toStdString();
		{
			SpinLock::ScopedLockType lock(mutex);
			if (launchpad_list.empty()) LED_timer.startTimer(10);
			launchpad_list.insert(this);
			LED_grid_current = vector<vector<VRGB>>(LED_w, vector<VRGB>(LED_h));
			LED_grid_target = vector<vector<VRGB>>(LED_w, vector<VRGB>(LED_h));
			LED_raw_data = make_unique<uint8[]>(static_cast<size_t>(LED_w) * LED_h * 5 + 6);
			copy_n("00'20'29'02'0D'03"_hex, 6, LED_raw_data.get()); //기본 명령어 헤더
			automatic_transmission = true;
		}
	}
	launchpad::~launchpad()
	{
		{
			SpinLock::ScopedLockType lock(mutex);
			launchpad_list.erase(this);
			if (launchpad_list.empty()) LED_timer.stopTimer();
		}
		deviceManager.reset();
	}

	void launchpad::send_message_now(MidiMessage& message)
	{
		output->sendMessageNow(message);
	}

	void launchpad::send_hex(const String& hex)
	{
		auto k = hexStringToBytes(hex);
		auto m = MidiMessage::createSysExMessage(&k[0], static_cast<int>(k.size()));
		send_message_now(m);
	}

	void launchpad::send_hex(const uint8* hex, size_t length)
	{
		auto m = MidiMessage::createSysExMessage(hex, static_cast<int>(length));
		send_message_now(m);
	}

	void launchpad::send_LED()
	{
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

	void launchpad::set_rgb(uint8 x, uint8 y, uint8 r, uint8 g, uint8 b)
	{
		if (LED_w < x || LED_h < y) throw range_error("x 또는 y 범위 오류");
		SpinLock::ScopedLockType lock(mutex);
		LED_grid_target[x][y] = VRGB{ 0xFF,static_cast<uint8>(r >> 1),static_cast<uint8>(g >> 1),static_cast<uint8>(b >> 1) };
		if (!automatic_transmission)
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

	void launchpad::set_velocity(uint8 x, uint8 y, uint8 v)
	{
		if (LED_w < x || LED_h < y) throw range_error("x 또는 y 범위 오류");
		SpinLock::ScopedLockType lock(mutex);
		LED_grid_target[x][y] = VRGB{ v,0,0,0 };
		if (!automatic_transmission)
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

	void launchpad::set_porgream_mode(bool flag)
	{
		auto t = "00'20'29'02'0D'0E'01"_hex;
		auto f = "00'20'29'02'0D'0E'00"_hex;
		MidiMessage 메시지 = MidiMessage::createSysExMessage(flag ? t : f, 7);
		send_message_now(메시지);
	}

	Array<MidiDeviceInfo> launchpad::get_available_list()
	{
		Array<MidiDeviceInfo> devices;
		auto id_list = map<string, uint8_t>();
		//auto availableDevices = MidiInput::getAvailableDevices();
		auto availableDevices = MidiOutput::getAvailableDevices();

		for (auto& deviceInfo : availableDevices)
		{
			auto& identifier = deviceInfo.identifier;

			if (!identifier.startsWith("\\\\?\\usb#")) continue;

			auto pos = 0;

			pos = identifier.indexOf(8, "vid_");
			const auto& vid = 0 <= pos ? identifier.substring(pos + 4, pos + 8).toStdString() : "0000";

			pos = identifier.indexOf(pos + 9, "pid_");
			const auto& pid = 0 <= pos ? identifier.substring(pos + 4, pos + 8).toStdString() : "0000";

			//허용하는 런치패드인지 확인
			auto it = VPID_map.find(vid + pid);
			if (it == VPID_map.end()) continue; //지원되는 런치패드 아님.

			pos = identifier.indexOf(pos + 9, "{");
			const auto& id = 0 <= pos ? identifier.substring(pos + 1, identifier.indexOf(pos + 1, "}")).toStdString() : "0";

			auto [id_list_it, success] = id_list.try_emplace(id, 0_uc);
			if (!success) ++(id_list_it->second);

			if (id_list_it->second != get<1>(it->second)) continue; //중복 건너뛰기
			deviceInfo.name = get<0>(it->second);

			//cout << vid << " " << pid << " " << id << " -> " << deviceInfo.name << endl;
			//cout << deviceInfo.identifier << endl;
			devices.add(deviceInfo);
		}

		return devices;
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

	set<launchpad*> launchpad::launchpad_list = set<launchpad*>();
	SpinLock launchpad::mutex = SpinLock();
	launchpad::LED_global_timer launchpad::LED_timer = launchpad::LED_global_timer();

	void launchpad::LED_global_timer::hiResTimerCallback()
	{
		SpinLock::ScopedLockType lock(mutex);
		for (auto& l : launchpad_list)
		{
			if (!l->automatic_transmission) continue;
			l->send_LED();
		}
		//printf("!");
	}
}