// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "core.h"
#include "audio.h"
#include "launchpad.h"

namespace uniq
{
	class timeline_cue : public ID<timeline_cue>, public hierarchy::hierarchy_feature
	{
	public:
		using cue_point_t = std::chrono::duration<int64_t, std::micro>;
	protected:
		explicit timeline_cue(cue_point_t cue_point);
	public:
		chain<cue_point_t> cue_point {this, std::chrono::microseconds(0)};
		auto operator<=>(const timeline_cue &other) const;
	};

	struct timeline_group : ID<timeline_group>, hierarchy::hierarchy_feature
	{
		using press_duration_t = timeline_cue::cue_point_t;
		chain<int8_t> button_x{this, 0};
		chain<int8_t> button_y{this, 0};
		press_duration_t press_duration {0};
		std::shared_ptr<audio_segment> segment;
		std::shared_ptr<timeline_cue> start_cue;
		// std::shared_ptr<LED> led; //TODO: LED 클래스 구현 후 추가
	};

	class timeline : public ID<timeline>//, public hierarchy::hierarchy_feature
	{
	public:
		using cue_point_t = timeline_cue::cue_point_t;
		struct timeline_group_compare_start_cue
		{
			using is_transparent = void;
			bool operator()(const std::shared_ptr<timeline_group>& lhs, const std::shared_ptr<timeline_group>& rhs) const;
			bool operator()(const std::shared_ptr<timeline_group>& lhs, const cue_point_t& rhs) const;
			bool operator()(const cue_point_t& lhs, const std::shared_ptr<timeline_group>& rhs) const;
		};
		// struct key_info
		// {
		// 	// struct key_sound
		// 	// {
		// 	// 	uint8_t chain_num = 0;
		// 	// 	uint8_t chain_num2 = 0;
		// 	// 	uint8_t x = 0;
		// 	// 	uint8_t y = 0;
		// 	// 	std::shared_ptr<audio_segment> segment;
		// 	// };
		// 	const uint8_t w_ = 8;
		// 	const uint8_t h_ = 8;
		// 	// uint8_t current_chain_num = 0;
		// 	// std::vector<uint8_t> current_chain_num2; // [chain_num]
		// 	// std::vector<std::vector<bool>> is_pressed; // [y][x]
		//
		// };
	private:
		struct group_callback
		{
			std::shared_ptr<timeline_group> group;
			std::vector<int> button_x_callback_id_list;
			std::vector<int> button_y_callback_id_list;
			std::vector<int> start_cue_callback_id_list;
		};
		struct group_callback_set_compare
		{
			using is_transparent = void;
			bool operator()(const std::shared_ptr<group_callback>& lhs, const std::shared_ptr<group_callback>& rhs) const;
			bool operator()(const std::shared_ptr<group_callback>& lhs, const std::shared_ptr<timeline_group>& rhs) const;
			bool operator()(const std::shared_ptr<timeline_group>& lhs, const std::shared_ptr<group_callback>& rhs) const;
			bool operator()(const std::shared_ptr<group_callback>& lhs, const cue_point_t& rhs) const;
			bool operator()(const cue_point_t& lhs, const std::shared_ptr<group_callback>& rhs) const;
		};
		std::string name_;
		std::set<std::shared_ptr<group_callback>, group_callback_set_compare> group_callback_set_;
		std::vector<std::vector<std::set<std::shared_ptr<timeline_group>, timeline_group_compare_start_cue>>>
		key_group_grid_; // [x][y]
		// std::vector<std::vector<uint16_t>> press_count_; // [x][y]
		std::vector<std::vector<std::shared_ptr<timeline_group>>> last_play_group_grid_; //[x][y]
	protected:
		explicit timeline(std::string name);
		const uint8_t w_ = 10;
		const uint8_t h_ = 10;
	public:
		struct internal
		{
			timeline* timeline_;
			explicit internal(timeline* timeline);
			[[nodiscard]]
			auto key_group_get(uint8_t x, uint8_t y) const -> std::set<std::shared_ptr<timeline_group>, timeline_group_compare_start_cue>&;
			[[nodiscard]]
			auto group_callback_set_get() const -> std::set<std::shared_ptr<group_callback>, group_callback_set_compare>&;
		} internal{this};
		[[nodiscard]] std::string name_get() const;
		void name_set(const std::string& name);
		bool group_add(const std::shared_ptr<timeline_group>& group);
		bool group_remove(const std::shared_ptr<timeline_group>& group);
		// auto group_get(uint8_t x, uint8_t y) -> std::shared_ptr<timeline_group>;
		// auto group_lower_bound(const std::shared_ptr<timeline_group>& group) -> std::set<std::shared_ptr<timeline_group>, timeline_group_compare_start_cue>::iterator;
		// auto group_find_ceil(const cue_point_t& cue) -> std::shared_ptr<timeline_group>;

		auto last_play_group_get(uint8_t x, uint8_t y) -> std::shared_ptr<timeline_group>;
		void last_play_group_set(uint8_t x, uint8_t y, const std::shared_ptr<timeline_group>& group);
		auto last_play_group_reset_all() -> void;
		// uint16_t press_count_get(uint8_t x, uint8_t y);
		// void press_count_increase(uint8_t x, uint8_t y);
		// void press_count_reset(uint8_t x, uint8_t y, cue_point_t cue = std::chrono::microseconds(0));
		// void play(uint8_t x, uint8_t y, cue_point_t cue = std::chrono::microseconds(0));

		// [[nodiscard]] const std::set<std::shared_ptr<timeline_group>, timeline_group_compare_start_cue>& group_set_get() const;
	};

	struct timeline_page : ID<timeline_page>
	{
		using cue_point_t = timeline_cue::cue_point_t;
		std::shared_ptr<timeline_cue> start_cue;
		struct xy
		{
			int8_t x = 0;
			int8_t y = 0;
			xy(int8_t x, int8_t y);
			xy(uint8_t x, uint8_t y);
			xy(int x, int y);
			auto operator<=>(const xy& other) const = default;
		};
		struct set_compare
		{
			using is_transparent = void;
			bool operator()(const std::shared_ptr<timeline_page>& lhs, const std::shared_ptr<timeline_page>& rhs) const;
			bool operator()(const std::shared_ptr<timeline_page>& lhs, const cue_point_t& rhs) const;
			bool operator()(const cue_point_t& lhs, const std::shared_ptr<timeline_page>& rhs) const;
		};
		std::map<xy, std::shared_ptr<timeline_page>> next_page_map;
		explicit timeline_page(cue_point_t cue);
		bool next_page_set(const std::shared_ptr<timeline_page>& timeline_page, xy xy);
		auto next_page_get(xy xy) -> std::shared_ptr<timeline_page>;
		bool next_page_remove(xy xy);
	};

	class uniq : public ID<uniq>
	{
	public:
		using cue_point_t = timeline_cue::cue_point_t;
	private:
		struct page_callback
		{
			std::shared_ptr<timeline_page> page;
			std::vector<int> callback_id_list;
		};
		struct page_set_compare
		{
			using is_transparent = void;
			bool operator()(const std::shared_ptr<page_callback>& lhs, const std::shared_ptr<page_callback>& rhs) const;
			bool operator()(const std::shared_ptr<page_callback>& lhs, const std::shared_ptr<timeline_page>& rhs) const;
			bool operator()(const std::shared_ptr<timeline_page>& lhs, const std::shared_ptr<page_callback>& rhs) const;
			bool operator()(const std::shared_ptr<page_callback>& lhs, const cue_point_t& rhs) const;
			bool operator()(const cue_point_t& lhs, const std::shared_ptr<page_callback>& rhs) const;
		};
		std::string title_;
		std::string producer_name_;
		std::vector<std::shared_ptr<timeline>> timeline_list_;
		std::vector<std::shared_ptr<audio_source>> audio_source_list_;
		std::shared_ptr<audio_player> player_ = audio_player::create();
		std::shared_ptr<launchpad> launchpad_;
		int launchpad_callback_id_{-1};
		int launchpad_button_down_callback_id_{-1};
		int launchpad_button_up_callback_id_{-1};
		std::set<std::shared_ptr<page_callback>, page_set_compare> page_set_;
		std::shared_ptr<timeline_page> current_page_;
		class guide_timer : public juce::HighResolutionTimer
		{
			uniq* uniq_;
		public:
			explicit guide_timer(uniq* uniq);
			void hiResTimerCallback() override;
		} guide_timer_{this};
		std::chrono::milliseconds guide_timer_interval_{10};
		bool guide_start_{false};
		bool guide_play_{false};
		const cue_point_t guide_simul_ = std::chrono::milliseconds(20);
		struct guide_color
		{
			uint8_t r {0x3F};
			uint8_t g {0x00};
			uint8_t b {0x7F};
		} guide_color_;
		struct guide_group
		{
			std::shared_ptr<timeline_group> group;
			bool is_played{false};
			// std::shared_ptr<timeline_page> page;
		};
		std::deque<guide_group> guide_group_deque_;
		cue_point_t guide_cue_{0}; // 현재 가이드 위치
		cue_point_t guide_cue_step_{std::chrono::milliseconds(500)}; // 가이드 이동 간격
		cue_point_t guide_play_cue_{0}; // 가이드 재생 시작 위치
		// std::chrono::steady_clock::time_point guide_start_play_time_;
		// std::chrono::steady_clock::time_point guide_pause_time_;
		// std::chrono::time_point<std::chrono::system_clock> guide_button_down_time_;
		std::chrono::steady_clock::time_point guide_toggle_button_down_time_;
		std::chrono::milliseconds guide_toggle_press_duration_{300};
		// std::shared_ptr<timeline_page> guide_page_;

		void guide_update(bool play_audio_flag = false);
		void guide_togle();
		bool guide_button_down_check(uint8_t x, uint8_t y);
		bool guide_button_up_check(uint8_t x, uint8_t y);
		// std::shared_ptr<pad_key_info> pad_key_info_ = std::make_shared<pad_key_info>();

		void audio_play(const std::shared_ptr<timeline>& target_timeline, const std::shared_ptr<timeline_group>& target_group);
	protected:
		uniq();
	public:
		~uniq();
		[[nodiscard]] std::string title_get() const;
		void title_set(const std::string& title);
		[[nodiscard]] std::string producer_name_get() const;
		void producer_name_set(const std::string& producer_name);
		[[nodiscard]] std::shared_ptr<audio_player> player_get() const;
		bool audio_load(const std::string& path);
		struct internal
		{
			uniq* uniq_;
			explicit internal(uniq* uniq);
			// std::shared_ptr<pad_key_info>& pad_key_info{uniq_->pad_key_info_};

			[[nodiscard]] std::shared_ptr<audio_source> audio_load(std::unique_ptr<juce::InputStream> input_stream,
			                                                       const std::string &extension,
			                                                       const std::string &path = {},
			                                                       const std::string &name = {}) const;

			void launchpad_callback(const std::uint8_t* data, int size);
		} internal{this};
		void audio_source_add(const std::shared_ptr<audio_source> &audio_source);
		auto timeline_create(const std::string &name) -> std::shared_ptr<timeline>;
		auto timeline_get(const std::string &name) -> std::shared_ptr<timeline>;
		bool timeline_remove(const std::string &name);
		auto timeline_page_add(const std::shared_ptr<timeline_page> &page) -> bool;
		auto timeline_page_create(const cue_point_t &cue) -> std::shared_ptr<timeline_page>;
		auto timeline_page_find_floor(const cue_point_t &cue) -> std::shared_ptr<timeline_page>;
		auto timeline_page_remove(const std::shared_ptr<timeline_page> &page) -> bool;
		// std::shared_ptr<timeline> timeline_add(const std::shared_ptr<timeline> &timeline);
		// bool timeline_remove(const std::shared_ptr<timeline> &timeline);
		// std::shared_ptr<timeline_group> timeline_group_add(const std::shared_ptr<timeline_group> &group);
		// bool timeline_group_remove(const std::shared_ptr<timeline_group> &timeline_group);
		auto guide_start(const cue_point_t &cue = std::chrono::microseconds(0)) -> void;
		auto guide_resume(const cue_point_t &cue = std::chrono::microseconds(0)) -> void;
		auto guide_position_set(const cue_point_t &cue) -> void;
		auto guide_position_get() -> cue_point_t;
		auto guide_pause() -> void;
		auto guide_stop() -> void;
		bool launchpad_connect(const std::shared_ptr<launchpad> &launchpad);
		bool launchpad_auto_connect();
		bool launchpad_disconnect_all();
		void pad_button_down(std::uint8_t x, std::uint8_t y, std::uint8_t velocity);
		void pad_button_up(std::uint8_t x, std::uint8_t y);
		void pad_button_touch(std::uint8_t x, std::uint8_t y, std::uint8_t velocity);
	};
}
