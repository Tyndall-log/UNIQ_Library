// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "core.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h> // GPL-3.0-or-later

namespace uniq
{
	namespace internal
	{
		class audio_format_manager
		{
			inline static std::shared_ptr<juce::AudioFormatManager> format_manager_{};
		public:
			static std::shared_ptr<juce::AudioFormatManager> get();
		};

		struct audio_data
		{
			juce::AudioBuffer<float> buffer_;
			std::uint32_t sample_rate_ = 0;
			std::string extension_;
			std::string path_;
			std::string name_;
			shared_recursive_timed_mutex_priority mutex_;
		public:
			static std::shared_ptr<audio_data> load(const std::string& path);
		};

		struct fade_low_data
		{
			std::chrono::duration<std::int32_t, std::micro> start_time_;
			std::chrono::duration<std::int32_t, std::micro> duration_;
		};

		struct play_low_data
		{
			std::weak_ptr<audio_data> data_;
			std::chrono::microseconds target_time_;
			std::uint32_t start_sample_ = 0;
			std::uint32_t end_sample_ = -1;
			//TODO: 커스텀 함수 추가 (예: 베지어 곡선) 지금은 선형으로만 구현
		};

		struct audio_play_data : public play_low_data
		{
			// std::uint8_t group_ = 0;
			fade_low_data fade_in_;
			fade_low_data fade_out_;
			std::chrono::duration<std::int32_t, std::micro> sync_duration_start_; //일반적으로 음수(최대 +- 35분)
			std::chrono::duration<std::int32_t, std::micro> sync_duration_end_; //일반적으로 양수
			std::vector<play_low_data> sync_target_list_;
			std::vector<play_low_data> fade_out_target_list_; //일반적으로 1개만 사용
		};

		class audio_custom_source : public juce::AudioSource
		{
			struct play_data
			{
				std::weak_ptr<audio_data> data_;
				std::uint32_t start_position_ = 0;
				std::uint32_t end_position_ = 0;
				std::chrono::duration<std::uint32_t, std::micro> start_delay_;
				std::uint32_t start_sample_ = 0;
				std::uint32_t end_sample_ = -1;
				fade_low_data fade_in_;
				fade_low_data fade_out_;
				std::chrono::duration<std::int32_t, std::micro> sync_duration_start_; //일반적으로 음수(최대 +- 35분)
				std::chrono::duration<std::int32_t, std::micro> sync_duration_end_; //일반적으로 양수
			};

			struct play_data_compare
			{
				bool operator()(const std::unique_ptr<play_data>& a, const std::unique_ptr<play_data>& b) const
				{
					return a->end_position_ > b->end_position_;
				}
			};

			struct waiting_data_compare
			{
				bool operator()(const std::unique_ptr<play_data>& a, const std::unique_ptr<play_data>& b) const
				{
					return a->start_position_ > b->start_position_;
				}
			};

			std::priority_queue<std::unique_ptr<play_data>, std::vector<std::unique_ptr<play_data>>, play_data_compare> play_data_list_;
			std::priority_queue<std::unique_ptr<play_data>, std::vector<std::unique_ptr<play_data>>, waiting_data_compare> waiting_data_list_;
			spin_lock lock_;
			float speed_ = 1.0f;
			float gain_ = 1.0f;
			double sample_rate_ = 0;
			std::uint32_t sample_position_ = 0;

		public:
			void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
			void releaseResources() override;
			void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

			void add_audio(std::shared_ptr<audio_play_data> data);
		};
	}

	class audio_device : public ID<audio_device>
	{
	private:
	};

	class audio_segment;

	class audio_source : public ID<audio_source>, public hierarchy
	{
	public:
		class audio_cue : public ID<audio_cue>//, public hierarchy, callback_event<audio_cue>
		{
			std::uint64_t cue_ = 0;
			explicit audio_cue(std::uint64_t cue);
			void set(std::uint64_t cue);
		public:
			// bool try_set(std::uint64_t cue);
			[[nodiscard]] std::uint64_t get() const;
			std::strong_ordering operator<=>(const audio_cue& other) const;
			explicit operator std::uint64_t() const;
		};
	private:
		struct audio_cue_compare
        {
            bool operator()(const std::shared_ptr<audio_cue>& a, const std::shared_ptr<audio_cue>& b) const
            {
                return *a < *b;
            }
        };

		struct audio_segment_compare
		{
			bool operator()(const std::shared_ptr<audio_segment>& a, const std::shared_ptr<audio_segment>& b) const
			{
				return a < b; //TODO: 구현
			}
		};

		std::shared_ptr<internal::audio_data> data_;
		std::set<std::shared_ptr<audio_cue>, audio_cue_compare> cue_point_list_;
		std::set<std::shared_ptr<audio_segment>> segment_list_;


	protected:
		audio_source();

	public:
		static std::shared_ptr<audio_source> audio_load(const std::string& path);
		bool cue_add(std::uint64_t cue);
		bool cue_remove(std::uint64_t cue);
	};

	class audio_segment : public ID<audio_segment>, public callback_event<audio_segment>
	{
		// std::we
		std::shared_ptr<audio_source::audio_cue> start_cue_;
		void play();
	};

}