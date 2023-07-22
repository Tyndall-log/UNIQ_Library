// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "../Launchpad/launchpad.h"
#include "../system.h"

using namespace std;
using namespace juce;
using namespace uniq;

namespace uniq
{
	class A : public ID<A>
	{
	protected:
		A() = default;
	public:
		std::string get_name()
		{
			return "Class A";
		}
	};

	class B : public ID<B>
	{
	protected:
		B() = default;
	public:
		std::string get_name()
		{
			return "Class B";
		}
	};
}

int main()
{
	system("chcp 65001"); //한글 설정

	std::shared_ptr<A> a = A::create();
	std::shared_ptr<B> b = B::create();

	//객체 크기 확인
	std::cout << "sizeof(A): " << sizeof(A) << "\n";
	std::cout << "sizeof(B): " << sizeof(B) << "\n";
	std::cout << "sizeof(a): " << sizeof(a) << "\n";
	std::cout << "sizeof(b): " << sizeof(b) << "\n";

	auto a_from_ID = ID_manager::get_shared_ptr_by_ID<A>(a->get_ID()).value_or(nullptr);
	if (a_from_ID)
	{
		std::cout << "ID " << a->get_ID() << ": " << a_from_ID->get_name() << "\n";
	}

	auto b_from_ID = ID_manager::get_shared_ptr_by_ID<B>(b->get_ID()).value_or(nullptr);
	if (b_from_ID)
	{
		std::cout << "ID " << b->get_ID() << ": " << b_from_ID->get_name() << "\n";
	}

	auto test = ID_manager::get_shared_ptr_by_ID<B>(a->get_ID()).value_or(nullptr);
	if (test)
	{
		std::cout << "ID " << a->get_ID() << ": " << test->get_name() << "\n";
	}
	else
	{
		std::cout << "ID " << a->get_ID() << ": " << "nullptr" << "\n";
	}

	auto id_2 = 5;
	auto test2 = ID_manager::get_shared_ptr_by_ID<A>(id_2).value_or(nullptr);
	if (test)
	{
		std::cout << "ID " << id_2 << ": " << test->get_name() << "\n";
	}
	else
	{
		std::cout << "ID " << id_2 << ": " << "nullptr" << "\n";
	}

	return 0;
}

int main2()
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
		uint8 sysexdata[] = { 0x00,0x20,0x29,0x02,0x0D,0x08,static_cast<uint8>(127 - n) };
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


int main3()
{
	system("chcp 65001"); //한글 설정
	auto MMT = make_unique<MainMessageThread>(); //메시지 관리 스레드 시작
	auto ADM = make_shared<AudioDeviceManager>();

	String path(CharPointer_UTF8("C:/Users/eunsu/OneDrive/바탕 화면/김은수/unipad/음악/Look at Me !.wav"));

	// WAV 파일 경로 설정
	File wavFile(path);

	// AudioFormatManager 생성
	AudioFormatManager formatManager;

	// WAV 오디오 포맷 추가
	formatManager.registerFormat(new WavAudioFormat(), true);

	auto k = formatManager.createReaderFor(wavFile);

	// AudioFormatReader 생성
	std::unique_ptr<AudioFormatReader> reader(k);

	if (reader == nullptr)
	{
		std::cout << "Error: unable to read file!" << std::endl;
		return -1;
	}

	// AudioBuffer 생성
	AudioBuffer<float> buffer(reader->numChannels, reader->lengthInSamples);

	// AudioFormatReaderSource 생성
	std::unique_ptr<AudioFormatReaderSource> readerSource(new AudioFormatReaderSource(reader.get(), false));

	// AudioTransportSource 생성
	AudioTransportSource transportSource;

	// AudioTransportSource에 AudioFormatReaderSource 연결
	transportSource.setSource(readerSource.get());

	// AudioBuffer에 데이터 저장
	transportSource.getNextAudioBlock(AudioSourceChannelInfo(&buffer, 0, buffer.getNumSamples()));

	// AudioBuffer 출력
	//std::cout << buffer << std::endl;

	juce::AudioBuffer<float> audioBuffer(reader->numChannels, reader->lengthInSamples);

	reader->read(&audioBuffer, 0, reader->lengthInSamples, 0, true, true);

	int a;
	cin >>  a;

	return 0;
}
