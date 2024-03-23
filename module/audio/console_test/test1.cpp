// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "main.h"
#include <chrono>
#include <iostream>

// #include <juce_audio_basics/juce_audio_basics.h>
// #include <juce_audio_basics/utilities/juce_GenericInterpolator.h>
// #include <juce_audio_basics/utilities/juce_Interpolators.h>
// #include <juce_audio_basics/utilities/juce_IIRFilter.h>
#include <juce_audio_formats/juce_audio_formats.h> // GPL-3.0-or-later

using namespace std;
// using namespace juce;
using namespace uniq;

namespace uniq
{
	class audio_device_manager : public ID<audio_device_manager>
	{
		std::shared_ptr<message_thread> mt_ = message_thread::get();
		std::unique_ptr<juce::AudioDeviceManager> device_manager_;
	public:
		audio_device_manager()
		{
			log::println("audio_device_manager 생성자");
			const auto future = mt_->call_async([this] {
				device_manager_ = std::make_unique<juce::AudioDeviceManager>();
				device_manager_->initialiseWithDefaultDevices(0, 2);
			});
			log::println("AudioDeviceManager 초기화 중...");
			future.wait();
			log::println("AudioDeviceManager 초기화 완료");
		}
		~audio_device_manager()
		{
			log::println("audio_device_manager 소멸자");
			const auto future = mt_->call_async([this] {
				device_manager_.reset();
			});
			log::println("AudioDeviceManager 해제 중...");
			future.wait();
			log::println("AudioDeviceManager 해제 완료");
		}

		std::unique_ptr<juce::AudioDeviceManager>& get()
		{
			return device_manager_;
		}
	};


}

//static auto mt_ = message_thread::get();
//static auto mu_t = mutex_test::get();


int test1()
{
	int a;

	//juce::AudioDeviceManager deviceManager;
	audio_device_manager adm;
	auto& deviceManager = *adm.get();
	deviceManager.initialiseWithDefaultDevices(0, 2);

	juce::AudioFormatManager formatManager;
	formatManager.registerBasicFormats();
	juce::File file(audio_file_path.file1);
	auto reader = formatManager.createReaderFor(file);
	auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
	juce::AudioTransportSource transportSource;
	transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
	//transportSource.prepareToPlay(512, reader->sampleRate);
	juce::AudioSourcePlayer audioSourcePlayer;
	audioSourcePlayer.setSource(&transportSource);
	deviceManager.addAudioCallback(&audioSourcePlayer);

	transportSource.start();
	transportSource.setGain(0.5f);

	while (true)
	{
		std::string str;
		cin >> str;
		if (str == "stop")
		{
			transportSource.stop();
		}
		else if (str == "start")
		{
			transportSource.start();
		}
		else if (str == "pause")
		{
			transportSource.stop();
		}
		else if (str == "exit")
		{
			break;
		}
		else if (str == "pos")
		{
			cout << transportSource.getCurrentPosition() << endl;
			cout << transportSource.getLengthInSeconds() << endl;
			cin >> str;
			auto d = std::stod(str);
			if (d < 0) d = 0;
			if (d > transportSource.getLengthInSeconds()) d = transportSource.getLengthInSeconds();
			transportSource.setPosition(d);
		}
		else
		{
			cout << "stop, start, pause, exit 중 하나를 입력하세요." << endl;
			cout << "input: " << str << endl;
			juce::Thread::sleep(1000);
		}
	}
	//juce::JUCEApplicationBase::shutdown()
	return 0;
}
