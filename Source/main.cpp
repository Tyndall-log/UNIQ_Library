// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "test.h"
#include "system.h"

int main()
{
	system("chcp 65001"); //한글 설정
	auto MMT = make_unique<MainMessageThread>(); //메시지 관리 스레드 시작
	auto ADM = make_shared<AudioDeviceManager>();

	auto list = launchpad::get_available_list();
	for (auto& l : list)
	{
		cout << l.name << endl;
	}

	if (list.size() <= 0)
	{
		cout << "인식된 런치패드 없음." << endl;
		return 0;
	}
	unique_ptr<launchpad> lp = make_unique<launchpad>(ADM, list[0]);
	//lp->set_porgream_mode(true);

	//lp->get_list();
	auto start = "00h 20h 29h 02h 0Dh 03h"s;
	for (int i = 0, n = 0; n <= 255;)
	{
		//Thread::getCurrentThread()->sleep(1000);
		//cin >> i;
		cin.get();
		for (auto x : views::iota(0, 10))
			for (auto y : views::iota(0, 10))
				//lp->set_rgb((uint8)x, (uint8)y, 255 - n, n, 255 - n);
				lp->set_rgb((uint8)x, (uint8)y, 255 - n, 255 - n, 255 - n);
		//lp->send_hex(start + "03 0E 7F 7F 7F"s);
		//uint8 sysexdata[] = { 0x00,0x20,0x29,0x02,0x0D,0x03,(uint8)3,(uint8)11,0b0000'1000,0,0 };
		//uint8 sysexdata[] = { 0x00,0x20,0x29,0x02,0x0D,0x03,(uint8)3,(uint8)11,0b0000'0111,0,0 };
		//uint8 sysexdata[] = { 0x00,0x20,0x29,0x02,0x0D,0x03,(uint8)3,(uint8)11,0b0000'1000,0,0 };
		//uint8 sysexdata[] = { 0x00,0x20,0x29,0x02,0x0D,0x03,0x03,0x0E,0x,0,0 };
		uint8 sysexdata[] = { 0x00,0x20,0x29,0x02,0x0D,0x08,127 - n };
		MidiMessage 메시지 = MidiMessage::createSysExMessage(sysexdata, 6 + 5);
		lp->send_message_now(메시지);
		//auto l = "002029020D03000B0D010C1517020D"_hex;
		//MidiMessage 메시지 = MidiMessage::createSysExMessage("002029020D03000B0D010C1517020D"_hex, 15);
		n += 2;
	}
	//return 0;
	//lp->
	string a;
	cin >> a; //대기

	//메시지 보내기
	//uint8 sysexdata[] = { 0x7E, 0x7F, 0x06, 0x01 };
	//MidiMessage 메시지 = MidiMessage::createSysExMessage(sysexdata, 4);
	//auto l = "002029020D03000B0D010C1517020D"_hex;
	//MidiMessage 메시지 = MidiMessage::createSysExMessage("002029020D03000B0D010C1517020D"_hex, 15);
	//lp->send_message_now(메시지);
	//lp->set_porgream_mode(true);
	//lp->set_porgream_mode(false);
	//auto start = "00h 20h 29h 02h 0Dh 03h"s;
	//lp->send_hex(start + "03 0E 7F 7F 7F"s);

	//lp.reset();
	cin >> a; //대기
	return 0;
}
