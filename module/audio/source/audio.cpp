// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "audio.h"


using namespace std;
using namespace juce;

namespace uniq
{
	namespace internal
	{

		std::shared_ptr<AudioFormatManager> audio_format_manager::get()
		{
			if (!format_manager_)
			{
				format_manager_ = std::make_shared<AudioFormatManager>();
				format_manager_ -> registerBasicFormats();
			}
			return format_manager_;
		}

#pragma region audio_data

		shared_ptr<audio_data> audio_data::load(const std::string &path)
		{
			const auto format_manager = audio_format_manager::get();
			const auto reader = format_manager->createReaderFor(File(path));
			if (reader == nullptr)
			{
				log::println("audio_data::load: reader is nullptr");
				return nullptr;
			}
			auto buffer = AudioBuffer<float>(static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
			reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
			auto data = make_shared<audio_data>();
			data->buffer_ = buffer;
			data->sample_rate_ = static_cast<unsigned int>(reader->sampleRate);
			data->extension_ = File(path).getFileExtension().toStdString();
			data->path_ = path;
			data->name_ = File(path).getFileNameWithoutExtension().toStdString();
			return data;
		}

#pragma endregion audio_data

#pragma region audio_custom_source

		void audio_custom_source::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
		{
			// log::println("audio_custom_source::prepareToPlay");
			unique_lock lock(lock_);
			sample_rate_ = sampleRate;
			sample_position_ = 0;
		}

		void audio_custom_source::releaseResources()
		{
			log::println("audio_custom_source::releaseResources");
		}

		void audio_custom_source::getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill)
		{
			log::println("audio_custom_source::getNextAudioBlock");
			//https://docs.juce.com/master/classGenericInterpolator.html
			// AudioBuffer<float>
		}

		void audio_custom_source::add_audio(std::shared_ptr<audio_play_data> data)
		{
			unique_lock lock(lock_);

			if (!data->sync_target_list_.empty())
			{
			}


			auto pd = make_unique<play_data>();
		}
#pragma endregion audio_custom_source
	}

	audio_source::audio_cue::audio_cue(const std::uint64_t cue) : cue_(cue) {}

	void audio_source::audio_cue::set(std::uint64_t cue)
	{
		if (cue_ != cue)
		{
			// call_callback(*this, callback_mode::change_before);
			cue_ = cue;
			// call_callback(*this, callback_mode::change_after);
		}
	}

	std::uint64_t audio_source::audio_cue::get() const
	{
		return cue_;
	}

	std::strong_ordering audio_source::audio_cue::operator<=>(const audio_cue &other) const
	{
		return cue_ <=> other.cue_;
	}

	audio_source::audio_cue::operator std::uint64_t() const
	{
		return cue_;
	}

	audio_source::audio_source()
	{

	}

	std::shared_ptr<audio_source> audio_source::audio_load(const std::string& file_path)
	{
		const auto data = internal::audio_data::load(file_path);
		if (data == nullptr)
		{
			log::println("audio_source::audio_load: data is nullptr");
			return nullptr;
		}
		auto source = create();
		source->data_ = data;
		return source;
	}

	bool audio_source::cue_add(std::uint64_t cue)
	{
		if (!data_)
			return false;
		shared_lock lock(data_->mutex_);
		if (data_->buffer_.getNumSamples() <= cue)
			return false;
		// auto cue_point = make_shared<audio_cue>(cue);
		// auto result = cue_point_list_.insert(cue_point);
		// return result.second;
	}
}
