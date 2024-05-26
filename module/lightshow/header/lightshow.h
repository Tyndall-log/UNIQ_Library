// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "core.h"

namespace uniq::lightshow
{
	struct rgbav
	{
		union
		{
			struct
			{
				std::uint8_t r;
				union
				{
					struct
					{
						std::uint8_t g;
						std::uint8_t b;
					};
					struct
					{
						std::uint8_t off_flag : 1;
						std::uint8_t v_flag : 1;
						std::uint8_t : 0; // padding
						std::uint8_t v : 8;
					};
				};
				std::uint8_t a;
			};
			std::uint32_t all;
		};
		rgbav(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a);
		explicit rgbav(std::uint8_t v);
		explicit rgbav(std::uint32_t all);
		explicit rgbav();

		void rgba_set(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a);
		[[nodiscard]] auto rgba_get() const -> std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>;
		void velocity_set(std::uint8_t v);
		[[nodiscard]] auto velocity_get() const -> std::uint8_t;
		void off_set();
		void all_set(std::uint32_t all);
		[[nodiscard]] auto all_get() const -> std::uint32_t;
		bool operator==(const rgbav& rhs) const;
		bool operator!=(const rgbav& rhs) const;
		[[nodiscard]] bool is_velocity() const;
		[[nodiscard]] bool is_off() const;
	};

	class rgbav_sequence_grid
	{
	public:
		using sequence_time_t = std::chrono::milliseconds;
		struct rgbav_pair
		{
			sequence_time_t time;
			rgbav color;
		};
		struct rgbav_pair_compare
		{
			using is_transparent = void;
			bool operator()(const rgbav_pair& lhs, const rgbav_pair& rhs) const;
			bool operator()(const rgbav_pair& lhs, const sequence_time_t& rhs) const;
			bool operator()(const sequence_time_t& lhs, const rgbav_pair& rhs) const;
		};
		using rgbav_sequence = std::set<rgbav_pair, rgbav_pair_compare>;
	private:
		std::vector<std::vector<rgbav_sequence>> grid;
		std::uint8_t width;
		std::uint8_t height;
	public:
		rgbav_sequence_grid(std::uint8_t width, std::uint8_t height);
		bool rgbav_add(std::uint8_t x, std::uint8_t y, sequence_time_t time, rgbav color);
		bool rgbav_remove(std::uint8_t x, std::uint8_t y, sequence_time_t time);
		struct rgbav_pair_last
		{
			const rgbav_pair& pair;
			bool last;
		};
		void clear();
		[[nodiscard]] auto rgbav_pair_get(std::uint8_t x, std::uint8_t y, sequence_time_t time) const -> std::optional<rgbav_pair_last>;
		[[nodiscard]] auto rgbav_sequence_get(std::uint8_t x, std::uint8_t y) const -> std::optional<rgbav_sequence>;
		[[nodiscard]] auto width_get() const -> std::uint8_t;
		[[nodiscard]] auto height_get() const -> std::uint8_t;
	};

	class lightshow_data : public ID<lightshow_data>
	{
		using sequence_time_t = rgbav_sequence_grid::sequence_time_t;
		sequence_time_t start_time_;
		sequence_time_t duration_;
		uint8_t x_ = 0;
		uint8_t y_ = 0;
		uint8_t repeat_ = 1;
		rgbav_sequence_grid rgbav_sequence_grid_;
		double speed_ = 1.0; //아직 사용하지 않음
	public:
		explicit lightshow_data(const rgbav_sequence_grid& rgbav_sequence_grid, uint8_t repeat = 1, sequence_time_t start_time = sequence_time_t(0));
		auto rgbav_sequence_grid_get() -> rgbav_sequence_grid&;
		auto rgbav_sequence_grid_get(sequence_time_t time, uint8_t x, uint8_t y, uint8_t width, uint8_t height) const -> rgbav_sequence_grid;
		auto start_time_set(sequence_time_t start_time) -> void;
		[[nodiscard]] auto start_time_get() const -> sequence_time_t;
		[[nodiscard]] auto duration_get() const -> sequence_time_t;
		auto repeat_set(uint8_t repeat) -> void;
		[[nodiscard]] auto repeat_get() const -> uint8_t;
	};

	class lightshow_sequence : public ID<lightshow_sequence>
	{
		using sequence_time_t = rgbav_sequence_grid::sequence_time_t;
		sequence_time_t start_time_;
		std::vector<lightshow_data> lightshow_data_list_;
	public:
		lightshow_sequence();
		auto rgbav_sequence_grid_get(sequence_time_t time, uint8_t x, uint8_t y, uint8_t width, uint8_t height) -> rgbav_sequence_grid;
		auto lightshow_data_add(const std::shared_ptr<lightshow_data>& lightshow_data) -> bool;
		[[nodiscard]] auto lightshow_data_remove(const std::shared_ptr<lightshow_data>& lightshow_data) -> bool;
		auto sequence_time_set(sequence_time_t start_time) -> void;
		[[nodiscard]] auto sequence_time_get() const -> sequence_time_t;
	};

	class lightshow_layer : public ID<lightshow_layer>
	{
		using sequence_time_t = rgbav_sequence_grid::sequence_time_t;
		std::string layer_name_;
		std::vector<lightshow_sequence> lightshow_sequence_list_;
	public:
		lightshow_layer(std::string layer_name);
		[[nodiscard]] auto name_get() const -> std::string;
	};

	class lightshow : public ID<lightshow>
	{
		using sequence_time_t = rgbav_sequence_grid::sequence_time_t;
		struct rgbav_id
		{
			rgbav color;
			int id;
		};
		struct rgbav_id_time : rgbav_id
		{
			sequence_time_t time;
		};
		struct rgbav_id_time_compare
		{
			using is_transparent = void;
			bool operator()(const rgbav_id_time& lhs, const rgbav_id_time& rhs) const;
			bool operator()(const rgbav_id_time& lhs, const sequence_time_t& rhs) const;
			bool operator()(const sequence_time_t& lhs, const rgbav_id_time& rhs) const;
		};
		struct lightshow_pair
		{
			std::shared_ptr<lightshow_data> lightshow_data;
			sequence_time_t start_time;
		};
		const uint8_t width_ = 10;
		const uint8_t height_ = 10;
		using rgbav_id_array = std::array<std::array<rgbav_id, 10>, 10>;
		using rgbav_id_sequence = std::set<rgbav_id_time, rgbav_id_time_compare>;
		using rgbav_id_sequence_array = std::array<std::array<rgbav_id_sequence, 10>, 10>;
		rgbav_sequence_grid rgbav_id_sequence_ = rgbav_sequence_grid(width_, height_);
		lightshow_pair pad_lightshow_data_[10][10]; // [x][y]
		rgbav_id_sequence_array pad_rgbav_id_sequence_; // [x][y]
		rgbav_id_array pad_last_color_; // [x][y]
		std::chrono::steady_clock::time_point standard_time_; //기준 시간
		spin_lock lock_;
	public:
		struct internal
		{
			lightshow* lightshow_;
			explicit internal(lightshow* lightshow);
			using reset_flag_array = std::array<std::array<bool, 10>, 10>;
			rgbav_id_array rgbav_grid_current;
			rgbav_id_array rgbav_grid_target;
			reset_flag_array reset_flag;
		} internal{this};
		lightshow();
		auto standard_time_get() const -> std::chrono::steady_clock::time_point;
		auto lightshow_data_set(const std::shared_ptr<lightshow_data>& lightshow_data, uint8_t x, uint8_t y, sequence_time_t delay = sequence_time_t(0)) -> void;
		auto rgbav_array_get(sequence_time_t time) -> rgbav_id_array;
	};
}
