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
	class audio : public ID<audio>
	{
	private:

	};

	class audio_data : public ID<audio_data>
	{

	};

	class audio_device_manager : public ID<audio_device_manager>
	{
		std::shared_ptr<message_thread> mt_ = message_thread::get();
		std::unique_ptr<juce::AudioDeviceManager> device_manager_;
	public:
		audio_device_manager()
		{
			//메시지 스레드에서 AudioDeviceManager를 생성해야 한다.
			const auto future = mt_->call_async([this] {
				device_manager_ = std::make_unique<juce::AudioDeviceManager>();
				device_manager_->initialise(0, 2, nullptr, true);
			});
			future.wait();
		}
		~audio_device_manager()
		{
			//메시지 스레드에서 AudioDeviceManager를 파괴해야 한다.
			const auto future = mt_->call_async([this] {
				device_manager_.reset();
			});
			future.wait();
		}
	};
}

//static auto mt_ = message_thread::get();
//static auto mu_t = mutex_test::get();

//MainMessageThread MMT;
//static auto adm = std::make_unique<juce::AudioDeviceManager>(); //fail
//std::unique_ptr<juce::AudioDeviceManager> adm; //success

int test1()
{
	// //adm = std::make_unique<juce::AudioDeviceManager>(); //JUCE_ASSERT_MESSAGE_THREAD!!!
	//
	// //<<message_thread setting>>
	// MainMessageThread MMT;
	// juce::MessageManager::callAsync([] {
	// 	adm = std::make_unique<juce::AudioDeviceManager>(); //success
	// });

	//MainMessageThread MMT;
	// auto k = message_thread::get(); // 메시지 스레드 활성화
	// log::println("k.use_count() = " + std::to_string(k.use_count()));
	// {
	// 	//audio_device_manager adm;
	// 	//juce::AudioDeviceManager adm;
	// 	MainMessageThread MMT;
	// 	// //message_thread::activate();
	//	juce::MessageManager::callAsync([] {
	//		// log::println("callAsync");
	//		// auto adm = std::make_unique<juce::AudioDeviceManager>();
	//		// juce::Thread::sleep(1000);
	//		juce::MidiDeviceListConnection midiDeviceListConnection = juce::MidiDeviceListConnection::make([] {
	//			log::println("MidiDeviceListConnection");
	//		});
	//	});
	// 	// juce::Thread::sleep(2000);
	// 	//message_thread::deactivate();
	// }
	int a;
	// cin >> a;

	//message_thread::deactivate();
	//juce::AudioDeviceManager deviceManager;
	//

	juce::AudioDeviceManager deviceManager;

	//audio_device_manager adm;
	juce::AudioFormatManager formatManager;
	formatManager.registerBasicFormats();
	juce::File file(audio_file_path.file1);
	auto reader = formatManager.createReaderFor(file);
	deviceManager.initialiseWithDefaultDevices(0, 2);
	auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
	juce::AudioTransportSource transportSource;
	transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
	//transportSource.prepareToPlay(512, reader->sampleRate);
	juce::AudioSourcePlayer audioSourcePlayer;
	audioSourcePlayer.setSource(&transportSource);
	deviceManager.addAudioCallback(&audioSourcePlayer);

	cin >> a;
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

	//deviceManager.initialise(0, 2, nullptr, true);

	// // auto test_time = 1us + 1us;
	// // test_time += 1s;
	//
	// using duration1 = std::chrono::microseconds;
	// using duration2 = std::chrono::duration<int, std::ratio<1, 125000>>;
	//
	// duration1 d1(1);
	// duration2 d2(1);
	//
	// d1 += d2;
	// cout << d1.count() << endl;
	//
	// //대기
	// int a;
	// cin >> a;
	return 0;
}
