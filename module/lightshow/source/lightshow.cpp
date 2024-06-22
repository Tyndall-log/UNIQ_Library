// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "lightshow.h"

using namespace std;
using namespace juce;

namespace uniq::lightshow
{
	rgbav::rgbav(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a)
	{
		rgba_set(r, g, b, a);
	}

	rgbav::rgbav(const std::uint8_t v)
	{
		velocity_set(v);
	}

	rgbav::rgbav(const std::uint32_t all)
	{
		all_set(all);
	}

	rgbav::rgbav()
	{
		off_set();
	}

	void rgbav::rgba_set(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
	{
		if (a == 0)
		{
			this->all = 0;
			return;
		}
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	auto rgbav::rgba_get() const -> tuple<uint8_t, uint8_t, uint8_t, uint8_t>
	{
		if (is_velocity()) log::warn("velocity로 설정된 값을 rgba로 반환하고 있습니다.");
		return {this->r, this->g, this->b, this->a};
	}

	void rgbav::velocity_set(const uint8_t v)
	{
		this->off_flag = 0;
		this->v_flag = 1;
		this->v = v;
		this->a = 0;
	}

	auto rgbav::velocity_get() const -> uint8_t
	{
		if (!is_velocity()) log::warn("rgba로 설정된 값을 velocity로 반환하고 있습니다.");
		return this->v;
	}

	void rgbav::off_set()
	{
		this->off_flag = 1;
		this->v_flag = 1;
		this->v = 0;
		this->a = 0;
	}

	void rgbav::all_set(const uint32_t all)
	{
		this->all = all;
	}

	auto rgbav::all_get() const -> uint32_t
	{
		return this->all;
	}

	bool rgbav::operator==(const rgbav &rhs) const
	{
		return this->all == rhs.all;
	}

	bool rgbav::operator!=(const rgbav &rhs) const
	{
		return this->all != rhs.all;
	}

	bool rgbav::is_velocity() const
	{
		return a == 0 && v_flag == 1;
	}

	bool rgbav::is_off() const
	{
		return a == 0 && off_flag == 1;
	}

	bool rgbav_sequence_grid::rgbav_pair_compare::operator()(const rgbav_pair &lhs, const rgbav_pair &rhs) const
	{
		return lhs.time < rhs.time;
	}

	bool rgbav_sequence_grid::rgbav_pair_compare::operator()(const rgbav_pair &lhs, const sequence_time_t &rhs) const
	{
		return lhs.time < rhs;
	}

	bool rgbav_sequence_grid::rgbav_pair_compare::operator()(const sequence_time_t &lhs, const rgbav_pair &rhs) const
	{
		return lhs < rhs.time;
	}

	rgbav_sequence_grid::rgbav_sequence_grid(uint8_t width, uint8_t height) : width(width), height(height)
	{
		grid.resize(width);
		for (auto &i : grid)
		{
			i.resize(height);
		}
	}

	bool rgbav_sequence_grid::rgbav_add(const uint8_t x, const uint8_t y, const sequence_time_t time, const rgbav color)
	{
		if (width <= x || height <= y) return false;
		// grid[x][y].emplace(time, color);
		auto& grid_xy = grid[x][y];
		auto it = grid_xy.lower_bound(time);
		if (it != grid_xy.end() && it->time == time)
		{
			if (color.is_off()) return false;
			grid_xy.erase(it);
			// grid_xy.emplace(time, color);
			grid_xy.emplace(rgbav_pair{time, color});
		}
		// else grid_xy.emplace_hint(it, time, color);
		else grid_xy.emplace_hint(it, rgbav_pair{time, color});
		return true;
	}

	bool rgbav_sequence_grid::rgbav_remove(const uint8_t x, const uint8_t y, const sequence_time_t time)
	{
		if (width <= x || height <= y) return false;
		auto& grid_xy = grid[x][y];
		auto it = grid_xy.upper_bound(time);
		if (it == grid_xy.begin()) return false;
		grid_xy.erase(--it);
		return true;
	}

	void rgbav_sequence_grid::clear()
	{
		for (auto &i : grid)
		{
			for (auto &j : i)
			{
				j.clear();
			}
		}
	}

	auto rgbav_sequence_grid::rgbav_pair_get(const uint8_t x, const uint8_t y, const sequence_time_t time) const -> optional<rgbav_pair_last>
	{
		if (width <= x || height <= y) return std::nullopt;
		auto& grid_xy = grid[x][y];
		const auto it = grid_xy.upper_bound(time);
		if (it == grid_xy.begin()) return std::nullopt;
		return {{*prev(it), it == grid_xy.end()}};
	}

	auto rgbav_sequence_grid::rgbav_sequence_get(const std::uint8_t x, const std::uint8_t y) const -> optional<
		reference_wrapper<const rgbav_sequence>>
	{
		if (width <= x || height <= y) return std::nullopt;
		return grid[x][y];
	}

	auto rgbav_sequence_grid::width_get() const -> std::uint8_t
	{
		return width;
	}

	auto rgbav_sequence_grid::height_get() const -> std::uint8_t
	{
		return height;
	}

	lightshow_data::lightshow_data(const rgbav_sequence_grid& rgbav_sequence_grid, const uint8_t repeat, const sequence_time_t start_time)
	: start_time_(start_time), repeat_(repeat), rgbav_sequence_grid_(rgbav_sequence_grid)
	{
		//duration_ 계산
		auto rsg = rgbav_sequence_grid_;
		uint8_t width = rgbav_sequence_grid.width_get();
		uint8_t height = rgbav_sequence_grid.height_get();
		for (uint8_t i = 0; i < width; i++)
		{
			for (uint8_t j = 0; j < height; j++)
			{
				const auto& rgbav_sequence = rsg.rgbav_sequence_get(i, j);
				if (!rgbav_sequence.has_value()) continue;
				const auto& sequence = rgbav_sequence.value().get();
				if (sequence.empty()) continue;
				const auto&[time, color] = *sequence.rbegin();
				duration_ = max<sequence_time_t>(duration_, time);
			}
		}
	}

	auto lightshow_data::rgbav_sequence_grid_get(sequence_time_t time, uint8_t x, uint8_t y, uint8_t width,
	                                             uint8_t height) const -> rgbav_sequence_grid
	{
		rgbav_sequence_grid grid(width, height);
		// auto t = log::time("lightshow_data::rgbav_sequence_grid_get");
		const auto& rsg = rgbav_sequence_grid_;
		const uint8_t i_start = max<uint8_t>(0, x - x_);
		const uint8_t i_end = min<uint8_t>(rsg.width_get(), x + width - x_);
		const uint8_t j_start = max<uint8_t>(0, y - y_);
		const uint8_t j_end = min<uint8_t>(rsg.height_get(), y + height - y_);
		sequence_time_t _time = time - start_time_;
		if (_time < 0s) return grid;
		auto repeat = duration_.count() != 0 ? _time.count() / duration_.count() : 0;
		if (repeat_ != 0 && repeat_ <= repeat)
		{
			_time = duration_;
			repeat = repeat_ - 1;
		}
		else _time = duration_.count() != 0 ? _time % duration_ : _time;
		// log::info("time: " + to_string(_time.count()));
		for (uint8_t i = i_start; i < i_end; i++)
		{
			for (uint8_t j = j_start; j < j_end; j++)
			{
				// auto tt = log::time("for" + to_string(i) + to_string(j));
				const auto& sequence_o = rsg.rgbav_sequence_get(i, j);
				if (!sequence_o.has_value()) continue;
				const auto& sequence = sequence_o.value().get();
				if (sequence.empty()) continue;
				// tt->stamp("sequence");
				auto it = sequence.upper_bound(_time);
				if (it == sequence.begin())
				{
					if (repeat == 0) continue;
					it = sequence.end();
				}
				--it;
				grid.rgbav_add(i, j, start_time_ + duration_ * repeat + it->time, it->color);
			}
		}
		return grid;
	}

	auto lightshow_data::start_time_set(sequence_time_t start_time) -> void
	{
		start_time_ = start_time;
	}

	auto lightshow_data::start_time_get() const -> sequence_time_t
	{
		return start_time_;
	}

	auto lightshow_data::duration_get() const -> sequence_time_t
	{
		return duration_;
	}

	auto lightshow_data::repeat_set(uint8_t repeat) -> void
	{
		repeat_ = repeat;
	}

	auto lightshow_data::repeat_get() const -> uint8_t
	{
		return repeat_;
	}

	bool lightshow::rgbav_id_time_compare::operator()(const rgbav_id_time &lhs,
	                                                  const rgbav_id_time &rhs) const
	{
		return lhs.time < rhs.time;
	}

	lightshow::internal::internal(lightshow *lightshow) : lightshow_(lightshow)
	{
	}

	lightshow::lightshow(): standard_time_(chrono::steady_clock::now())
	{
	}

	auto lightshow::standard_time_get() const -> std::chrono::steady_clock::time_point
	{
		return standard_time_;
	}

	auto lightshow::lightshow_data_set(const std::shared_ptr<lightshow_data> &lightshow_data, const uint8_t x,
	                                   const uint8_t y, const sequence_time_t delay) -> void
	{
		const auto now = chrono::steady_clock::now();
		const auto start_time = chrono::duration_cast<sequence_time_t>(now - standard_time_) + delay;
		unique_lock lock(lock_);
		auto& pld_xy = pad_lightshow_data_[x][y];
		if (pld_xy.lightshow_data) // 이미 재생중인 경우
		{
			// 이전 값 off로 클리어
			auto pad_id = x * width_ + y;
			// auto& rgt = internal.rgbav_grid_target;
			// auto& rgt = pad_last_color_;
			for (uint8_t i = 0; i < width_; i++)
			{
				for (uint8_t j = 0; j < height_; j++)
				{
					if (pad_lightshow_last_color_[i][j].id == pad_id)
					{
						pad_lightshow_last_color_[i][j].color.off_set();
					}
				}
			}
		}
		pld_xy = {lightshow_data, start_time};
	}

	auto lightshow::rgbav_array_get(sequence_time_t time) -> rgbav_id_array
	{
		// rgbav_array array; 대신 pad_last_color_를 사용
		// pad_rgbav_id_sequence_ 초기화
		for (uint8_t i = 0; i < width_; i++)
		{
			for (uint8_t j = 0; j < height_; j++)
			{
				pad_rgbav_id_sequence_[i][j].clear();
			}
		}

		unique_lock lock(lock_);

		// // pad_lightshow_data_ 복사
		// lightshow_pair pad_lightshow_data[10][10];
		// // copy_n(&pad_lightshow_data_[0][0], 10 * 10, &pad_lightshow_data[0][0]);
		// for (uint8_t i = 0; i < width_; i++)
		// {
		// 	for (uint8_t j = 0; j < height_; j++)
		// 	{
		// 		const auto&[lightshow_data, start_time] = pad_lightshow_data_[i][j];
		// 		if (!lightshow_data) continue;
		// 		// 시간 범위 검사
		// 		if (time < start_time) continue;
		// 		auto end_time = start_time + lightshow_data->start_time_get() + lightshow_data->duration_get() * lightshow_data->repeat_get();
		// 		if (end_time < time)
		// 		{
		// 			pad_lightshow_data_[i][j] = {nullptr, 0s}; // 삭제
		// 			continue;
		// 		}
		// 		pad_lightshow_data[i][j] = pad_lightshow_data_[i][j];
		// 	}
		// }
		// lock.unlock();

		// auto t = log::time("rgbav_array_get");
		for (uint8_t i = 0; i < width_; i++)
		{
			for (uint8_t j = 0; j < height_; j++)
			{
				const auto&[lightshow_data, start_time] = pad_lightshow_data_[i][j];
				if (!lightshow_data) continue;
				// 시간 범위 검사
				if (time < start_time) continue;
				// pad_rgbav_id_sequence_에 추가
				const auto grid = lightshow_data->rgbav_sequence_grid_get(time - start_time, 0, 0, width_, height_);
				const auto pad_id = i * width_ + j;
				for (uint8_t x = 0; x < width_; x++)
				{
					for (uint8_t y = 0; y < height_; y++)
					{
						const auto& rso = grid.rgbav_sequence_get(x, y);
						if (!rso.has_value()) continue;
						const auto& rs = rso.value().get();
						if (rs.empty()) continue;
						const auto&[_time, color] = *rs.rbegin();
						pad_rgbav_id_sequence_[x][y].emplace(rgbav_id_time{color, pad_id, start_time + _time});
					}
				}
				auto end_time = start_time + lightshow_data->start_time_get() + lightshow_data->duration_get() * lightshow_data->repeat_get();
				if (end_time < time)
				{
					pad_lightshow_data_[i][j] = {nullptr, 0s}; // 삭제
				}
			}
		}

		// pad_last_color_ 채우기
		// lock.lock();
		for (uint8_t i = 0; i < width_; i++)
		{
			for (uint8_t j = 0; j < height_; j++)
			{
				const auto& rgbav_id_sequence = pad_rgbav_id_sequence_[i][j];
				if (rgbav_id_sequence.empty()) continue;
				for (const auto& rgbav_id_time : rgbav_id_sequence | views::reverse)
				{
					if (pad_lightshow_last_color_[i][j].time > rgbav_id_time.time) continue;
					if (pad_lightshow_last_color_[i][j].id == rgbav_id_time.id)
					{
						pad_lightshow_last_color_[i][j] = rgbav_id_time;
						break;
					}
					if (rgbav_id_time.color.is_off()) continue;
					pad_lightshow_last_color_[i][j] = rgbav_id_time;
					break;
				}
			}
		}
		lock.unlock();

		// // pad_last_color_ 출력(디버그 용)
		// string t;
		// for (uint8_t i = 0; i < width_; i++)
		// {
		// 	for (uint8_t j = 0; j < height_; j++)
		// 	{
		// 		const auto&[color, id] = pad_last_color_[i][j];
		// 		if (color.is_velocity())
		// 			t += to_string(color.velocity_get()) + " ";
		// 		else
		// 			t += "? ";
		// 	}
		// 	t += "\n";
		// }
		// log::info(t);

		// pad_last_color_에 색상 추가
		for (uint8_t i = 0; i < width_; i++)
		{
			for (uint8_t j = 0; j < height_; j++)
			{
				if (!pad_pressed_color_[i][j].color.is_off())
				{
					pad_last_color_[i][j] = {pad_pressed_color_[i][j].color, -2};
					continue;
				}
				if (!pad_guide_color_[i][j].color.is_off())
				{
					pad_last_color_[i][j] = {pad_guide_color_[i][j].color, -1};
					continue;
				}
				pad_last_color_[i][j] = {pad_lightshow_last_color_[i][j].color, pad_lightshow_last_color_[i][j].id};
			}
		}

		return pad_last_color_;
	}

	auto lightshow::guide_color_set(uint8_t x, uint8_t y, rgbav color, const int id) -> void
	{
		pad_guide_color_[x][y] = {color, id};
	}

	auto lightshow::pressed_color_set(uint8_t x, uint8_t y, rgbav color) -> void
	{
		pad_pressed_color_[x][y] = {color, -2};
	}

	// auto lightshow_sequence::rgbav_sequence_grid_get(sequence_time_t time, uint8_t x, uint8_t y, const uint8_t width,
	// const uint8_t height) -> rgbav_sequence_grid
	// {
	// 	rgbav_sequence_grid grid(width, height);
	// 	for (const auto& lightshow_data : lightshow_data_list_)
	// 	{
	// 		const auto& rgbav_sequence_grid = lightshow_data.rgbav_sequence_grid_get();
	// 		const auto& grid_o = rgbav_sequence_grid.rgbav_sequence_get(x, y);
	// 		if (!grid_o.has_value()) continue;
	// 		const auto& grid = grid_o.value();
	// 		for (const auto&[time, color] : grid)
	// 		{
	// 			if (time > this->start_time_ + time) break;
	// 			grid.rgbav_add(x, y, time, color);
	// 		}
	// 	}
	// 	return grid;
	// }
}
