// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include <ObjectArray.h>

#include "main.h"
#include <juce_audio_formats/juce_audio_formats.h> // GPL-3.0-or-later

namespace test_ns2
{
	using namespace uniq;
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

	class audio_device : public ID<audio_device>
	{
	private:

	};

	class audio_source : public ID<audio_source>
	{
		std::shared_ptr<juce::AudioBuffer<float>> buffer_;
		unsigned int sample_rate_ = 0;
		std::unique_ptr<juce::MemoryAudioSource> memory_source_;
		std::unique_ptr<juce::AudioTransportSource> transport_source_;
		std::unique_ptr<juce::AudioSourcePlayer> player_;
	public:
		// UNIQ_FUNC_CATEGORY_BEGIN(audio_source, buffer)
		// [[nodiscard]] std::shared_ptr<juce::AudioBuffer<float>> get() const
		// {
		// 	return parent_.buffer_;
		// }
		// UNIQ_FUNC_CATEGORY_END(buffer)
		explicit audio_source(const std::string &file_path)
		{
			juce::AudioFormatManager formatManager;
			formatManager.registerBasicFormats();
			const auto& file = juce::File(juce::CharPointer_UTF8(file_path.c_str()));
			const std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
			if (reader == nullptr)
			{
				log::println("오디오 파일 읽기 실패");
				return;
			}
			sample_rate_ = static_cast<unsigned int>(reader->sampleRate);
			buffer_ = std::make_shared<juce::AudioBuffer<float>>(static_cast<int>(reader->numChannels),
				static_cast<int>(reader->lengthInSamples));
			reader->read(buffer_.get(), 0,
				static_cast<int>(reader->lengthInSamples),
				0, true, true);
		}
		void play(audio_device_manager& adm)
		{
			auto& device_manager = *adm.get();
			memory_source_ = std::make_unique<juce::MemoryAudioSource>(*buffer_, false);
			transport_source_ = std::make_unique<juce::AudioTransportSource>();
			transport_source_->setSource(memory_source_.get(), 0, nullptr, sample_rate_);
			player_ = std::make_unique<juce::AudioSourcePlayer>();
			player_->setSource(transport_source_.get());
			player_->setGain(0.5f);
			device_manager.addAudioCallback(player_.get());
			transport_source_->start();
		}
		void speed(const float speed) const
		{
			const auto pos = transport_source_->getCurrentPosition();
			const auto len = transport_source_->getLengthInSeconds();
			transport_source_->setSource(memory_source_.get(), 0, nullptr, static_cast<double>(sample_rate_) * speed);
			const auto new_len = transport_source_->getLengthInSeconds();
			transport_source_->setPosition(pos * new_len / len);
			transport_source_->start();
		}

	private:
	};

	class audio_segment : public ID<audio_segment>
	{
	};

	class audio_cue : public ID<audio_cue>
	{
	};
}

int test2()
{
	CoInitialize(nullptr);

	using namespace test_ns2;
	auto mt = uniq::message_thread::get();
	audio_source as(audio_file_path.file2);

	audio_device_manager adm;
	int a;
	std::cin >> a;
	//as.play(adm);

	while(true)
	{
		std::string str;
		std::cin >> str;
		if (str == "speed")
		{
			float speed;
			std::cin >> speed;
			as.speed(speed);
		}
		else if (str == "exit")
		{
			break;
		}
	}

	//std::cout << "audio_source 크기: " << sizeof(uniq::audio_source) << std::endl;

	return 0;
}