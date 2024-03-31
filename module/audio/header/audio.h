// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "core.h"
#include "fade.h"
#include "interpolator.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h> // GPL-3.0-or-later

namespace uniq
{
	//전방 선언
	struct audio_segment;

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
			shared_recursive_timed_mutex_priority mutex_; //독점의 경우 가능한 1ms 이하로 잠궈야 함.
		public:
			static std::shared_ptr<audio_data> load(const std::string& path);
		};

		struct fade_low_data
		{
			inline static std::shared_ptr<audio::fade::i_fade<float>> default_fade_function_ = std::make_shared<audio::fade::fade_linear<float>>();

			std::chrono::duration<std::int32_t, std::micro> start_time_;
			// std::chrono::duration<std::int32_t, std::micro> duration_;
			std::chrono::duration<std::int32_t, std::micro> end_time_;
			std::shared_ptr<audio::fade::i_fade<float>> fade_function_ = default_fade_function_;
			bool reverse_ = false;
			[[nodiscard]] auto get_gain(const std::chrono::duration<std::int32_t, std::micro>& time) const -> float;
			[[nodiscard]] auto get_gain(float t) const -> float;
		};

		struct audio_play_data
		{
			id_t id_;
			std::weak_ptr<audio_data> data_;
			// std::uint8_t group_ = 0;
			std::uint32_t start_position_ = 0;
			std::uint32_t end_position_ = 0;
			std::uint32_t start_sample_ = 0;
			std::uint32_t end_sample_ = -1;
			fade_low_data fade_in_;
			fade_low_data fade_out_;
			std::chrono::microseconds target_time_;
			std::chrono::duration<std::int32_t, std::micro> sync_duration_start_; //일반적으로 음수(최대 +- 35분)
			std::chrono::duration<std::int32_t, std::micro> sync_duration_end_; //일반적으로 양수
			std::vector<id_t> sync_target_list_;
			std::vector<id_t> fade_out_target_list_; //일반적으로 1개만 사용
		};

		class audio_custom_source : public juce::AudioSource
		{
			using audio_position_t = std::uint32_t;
			using sample_position_t = std::uint32_t;
			struct play_data
			{
				id_t id_;
				static constexpr uint32_t padding = 8; //buffer_의 좌우 여유 공간(리샘플링 등으로 인한 오버플로 방지)
				// std::shared_ptr<audio_data> data_;
				// juce::AudioBuffer<float> buffer_;
				float **buffer_ = nullptr;
				std::uint32_t sample_rate_ = 48000;
				sample_position_t sample_num_ = 0;
				std::uint8_t channel_num_ = 0;
				double audio_start_position_delay_; //audio_position_t 단위 (0~1)
				audio_position_t audio_start_position_ = 0;
				audio_position_t audio_end_position_ = 0; //audio_start_position_delay_와 fade_out_ 포함 위치
				audio_position_t sync_start_position_ = 0; //data_의 시작 샘플 위치
				audio_position_t sync_end_position_ = 0; //data_의 끝 샘플 위치
				// sample_position_t start_sample_ = 0;
				// sample_position_t end_sample_ = 0;
				fade_low_data fade_in_;
				std::vector<fade_low_data> fade_out_list; //이월 가능
				float start_gain_ = 0.f; //fade_in_ 시작 시점의 게인(적용 길이에도 영향을 줌)
				float last_gain_ = 0.f; //fade_in_ 시작 시점의 게인(적용 길이에도 영향을 줌)
				struct next_s
				{
					id_t id_ = 0;
					std::shared_ptr<audio_data> data_;
					std::uint32_t sample_rate_ = 48000;
					std::chrono::duration<std::uint32_t, std::micro> pos_; //audio_start_position_ + audio_start_position_delay_에 해당하는 시간
					sample_position_t start_sample_ = 0;
					sample_position_t end_sample_ = 0xffffffffui32;
					bool exist_ = false; //true면 다음 데이터가 존재함(즉, fade_out_ 방지)
				};
				std::vector<next_s> next_list; //재생될(pos_) 순서대로 정렬(fade_out_.end_time_내에 있는 모든 데이터 필요)

				~play_data();
			};

			struct sync_playing_data_compare
			{
				bool operator()(const std::shared_ptr<play_data>& a, const std::shared_ptr<play_data>& b) const
				{
					return a->sync_end_position_ < b->sync_end_position_;
				}
			};

			struct sync_waiting_data_compare
			{
				bool operator()(const std::shared_ptr<play_data>& a, const std::shared_ptr<play_data>& b) const
				{
					return a->sync_start_position_ < b->sync_start_position_;
				}
			};

			struct playing_data_compare
			{
				bool operator()(const std::shared_ptr<play_data>& a, const std::shared_ptr<play_data>& b) const
				{
					return a->audio_end_position_ < b->audio_end_position_;
				}
			};

			struct waiting_data_compare
			{
				bool operator()(const std::shared_ptr<play_data>& a, const std::shared_ptr<play_data>& b) const
				{
					return a->audio_start_position_ < b->audio_start_position_;
				}
			};

			std::unordered_map<id_t, std::set<std::shared_ptr<play_data>, sync_playing_data_compare>> sync_data_map_;
			std::set<std::shared_ptr<play_data>, sync_playing_data_compare> sync_playing_data_set_;
			std::set<std::shared_ptr<play_data>, sync_waiting_data_compare> sync_waiting_data_set_;
			std::set<std::shared_ptr<play_data>, playing_data_compare> playing_data_set_; //재생했고, 끝나지 않은 것
			std::set<std::shared_ptr<play_data>, waiting_data_compare> waiting_data_set_; //재생하지 않은 것
			spin_lock sl_;
			juce::AudioBuffer<float> buffer_;
			// std::vector<std::vector<float>> low_buffer_;
			std::unique_ptr<float[]> low_buffer_;
			float target_speed_ = 1.0f;
			float gain_ = 1.0f;
			double sample_rate_ = 0;
			audio_position_t next_sample_position_ = 0; //getNextAudioBlock의 다음 호출에서 시작할 위치

			/// @brief 동기화 목록 갱신
			/// @warning 반드시 sl_ 잠금 상태에서 호출
			void sync_refresh();

			/// @brief 재생할 데이터를 갱신하고, 재생할 데이터를 정리해서 반환
			/// @warning 반드시 sl_ 잠금 상태에서 호출
			[[nodiscard]]
			std::unique_ptr<std::vector<std::vector<std::shared_ptr<play_data>>>> play_refresh();

			//반드시 sl_ 잠금 상태에서 호출
			void buffer_ready(const int &target_channel_num, const int &target_sample_num);

			void chennal_mapping_legacy(std::shared_ptr<play_data>& pd, juce::AudioBuffer<float> &data_buffer, const int &target_channel_num, const int &
			                     target_sample_num);
			/// @brief 재생할 데이터를 버퍼에 쓰기
			/// @return padding이 제외한 시작 위치(padding에 따라 음수 접근이 가능)
			const float** chennal_mapping();

		public:
			void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
			void releaseResources() override;
			void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
			void getNextAudioBlockLegacy(const juce::AudioSourceChannelInfo& bufferToFill);

			struct add_audio_param
			{
				struct
				{
					std::uint32_t start = 0;
					std::uint32_t end = 0xffffffffui32;
				} sample;
				struct
				{
					struct
					{
						std::chrono::microseconds time;
						std::vector<id_t> list;
					} target;
					struct
					{
						std::chrono::duration<std::int32_t, std::micro> start{}; //일반적으로 음수(최대 +- 35분)
						std::chrono::duration<std::int32_t, std::micro> end{}; //일반적으로 양수
					} duration;
				} sync;
				struct
				{
					fade_low_data in{};

					// struct out_data
					// {
					// 	fade_low_data fade{};
					//
					// };
					// std::vector<fade_low_data> out_list; // out.end_time_전에 재생

					struct out_list_compare
					{
						bool operator()(const fade_low_data& a, const fade_low_data& b) const
						{
							return a.end_time_ < b.end_time_;
						}
					};

				} fade;
			};
			auto add_audio(const std::shared_ptr<audio_data>& data, id_t id, add_audio_param param = {}) -> bool;
		};
	}

	class audio_device : public ID<audio_device>
	{
	private:
	};

	class audio_cue : public ID<audio_cue>, callback_event<audio_cue>, callback_check_event<audio_cue>//, public hierarchy
	{
	protected:
		std::uint64_t cue_ = 0;
		explicit audio_cue(std::uint64_t cue);
		void set(std::uint64_t cue);
	public:
		bool try_set(std::uint64_t cue);
		[[nodiscard]] std::uint64_t get() const;
		std::strong_ordering operator<=>(const audio_cue& other) const;
		std::strong_ordering operator<=>(const std::uint64_t& other) const;
		explicit operator std::uint64_t() const;
	};

	class audio_source : public ID<audio_source>, public hierarchy
	{
	private:
		struct audio_cue_compare
        {
			using is_transparent = void;
            bool operator()(const std::shared_ptr<audio_cue>& a, const std::shared_ptr<audio_cue>& b) const;
			bool operator()(const std::shared_ptr<audio_cue>& a, const std::uint64_t& b) const;
			bool operator()(const std::uint64_t& a, const std::shared_ptr<audio_cue>& b) const;
        };

		struct audio_segment_compare_start_cue
		{
			using is_transparent = void;
			bool operator()(const std::shared_ptr<audio_segment>& a, const std::shared_ptr<audio_segment>& b) const;
			bool operator()(const std::shared_ptr<audio_segment>& a, const std::uint64_t& b) const;
			bool operator()(const std::uint64_t& a, const std::shared_ptr<audio_segment>& b) const;
		};

		std::shared_ptr<internal::audio_data> data_;
		std::set<std::shared_ptr<audio_cue>, audio_cue_compare> cue_point_list_; //항상 2개 이상(시작, 끝)
		std::set<std::shared_ptr<audio_segment>, audio_segment_compare_start_cue> segment_start_set_;

		enum class cue_add_mode : std::uint8_t
		{
			segment_split_not_allowed, // 분할 비허용
			segment_split_keep_front, // 분할 허용, 기존 세그먼트 앞쪽에 유지
			segment_split_keep_back, // 분할 허용, 기존 세그먼트 뒷쪽에 유지
		};

		enum class cue_remove_mode : std::uint8_t
		{
			segment_merge_not_allowed, // 병합 비허용
			segment_merge_remove_front, // 병합 허용, 앞쪽 세그먼트 제거
			segment_merge_remove_back, // 병합 허용, 뒷쪽 세그먼트 제거
			segment_merge_leave_both, // 병합 허용, 두 세그먼트 모두 유지 (비추천)
		};
	protected:
		audio_source();

	public:
		static auto audio_load(const std::string &file_path) -> std::shared_ptr<audio_source>;
		template<cue_add_mode = cue_add_mode::segment_split_keep_front>
		auto cue_add(std::uint64_t cue) -> bool;
		auto cue_lower_bound(std::uint64_t cue) -> std::shared_ptr<audio_cue>;
		template<cue_remove_mode = cue_remove_mode::segment_merge_remove_back>
		auto cue_remove(std::uint64_t cue) -> bool;
	};

	struct audio_segment : ID<audio_segment>, callback_event<audio_segment>//, callback_check_event<audio_segment>
	{
		spin_lock sl_;
		std::weak_ptr<audio_source> source_;
		std::shared_ptr<audio_cue> start_cue_;
		// callback_event<audio_cue> start_cue_event_;
		std::shared_ptr<audio_cue> end_cue_;
		// callback_event<audio_cue> end_cue_event_;
		// void play();

		// std::chrono::microseconds sync_target_time_;
		std::vector<id_t> sync_target_list_;
		std::chrono::duration<std::int32_t, std::micro> sync_duration_start_; //일반적으로 음수(최대 +- 35분)
		std::chrono::duration<std::int32_t, std::micro> sync_duration_end_; //일반적으로 양수
		internal::fade_low_data fade_in_;
		internal::fade_low_data fade_out_;
		std::vector<id_t> fade_out_target_list_; //일반적으로 1개만 사용

	public:
		void start_cue_change(const std::shared_ptr<audio_cue>& cue);
		void end_cue_change(const std::shared_ptr<audio_cue>& cue);
	};
}

#include "audio.hpp"
