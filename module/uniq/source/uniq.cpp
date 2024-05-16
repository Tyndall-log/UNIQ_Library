// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "uniq.h"

#include <utility>

using namespace std;
using namespace juce;

namespace uniq
{
	auto timeline_cue::operator<=>(const timeline_cue &other) const
	{
		// if (const auto cmp = start_cue.get() <=> other.start_cue.get(); cmp != 0) return cmp;
		// return ID_get() <=> other.ID_get();
		return cue_point.get() <=> other.cue_point.get();
	}

	timeline_cue::timeline_cue(cue_point_t cue_point): cue_point(this, cue_point)
	{
	}

	bool timeline::timeline_group_compare_start_cue::operator()(const std::shared_ptr<timeline_group> &lhs,
																const std::shared_ptr<timeline_group> &rhs) const
	{
		const auto &lhs_start_cue = lhs->start_cue->cue_point.get();
		const auto &rhs_start_cue = rhs->start_cue->cue_point.get();
		if (lhs_start_cue != rhs_start_cue) return lhs_start_cue < rhs_start_cue;
		return lhs < rhs;
	}

	bool timeline::timeline_group_compare_start_cue::operator()(const std::shared_ptr<timeline_group> &lhs,
		const cue_point_t &rhs) const
	{
		return lhs->start_cue->cue_point.get() < rhs;
	}

	bool timeline::timeline_group_compare_start_cue::operator()(const cue_point_t &lhs,
		const std::shared_ptr<timeline_group> &rhs) const
	{
		return lhs < rhs->start_cue->cue_point.get();
	}

	bool timeline::group_callback_set_compare::operator()(const std::shared_ptr<group_callback> &lhs,
	                                                      const std::shared_ptr<group_callback> &rhs) const
	{
		if (const auto cmp = *lhs->group->start_cue->cue_point <=> *rhs->group->start_cue->cue_point; cmp != 0)
			return cmp < 0;
		return lhs->group->ID_get() < rhs->group->ID_get();
	}

	bool timeline::group_callback_set_compare::operator()(const std::shared_ptr<group_callback> &lhs,
		const std::shared_ptr<timeline_group> &rhs) const
	{
		return *lhs->group->start_cue->cue_point < *rhs->start_cue->cue_point;
	}

	bool timeline::group_callback_set_compare::operator()(const std::shared_ptr<timeline_group> &lhs,
		const std::shared_ptr<group_callback> &rhs) const
	{
		return *lhs->start_cue->cue_point < *rhs->group->start_cue->cue_point;
	}

	bool timeline::group_callback_set_compare::operator()(const std::shared_ptr<group_callback> &lhs,
		const cue_point_t &rhs) const
	{
		return *lhs->group->start_cue->cue_point < rhs;
	}

	bool timeline::group_callback_set_compare::operator()(const cue_point_t &lhs,
		const std::shared_ptr<group_callback> &rhs) const
	{
		return lhs < *rhs->group->start_cue->cue_point;
	}

	timeline::timeline(std::string name) : name_(move(name))
	{
		last_play_group_grid_ = vector(w_, vector<shared_ptr<timeline_group>>(h_, nullptr));
		key_group_grid_ = vector(w_, vector(h_, set<shared_ptr<timeline_group>, timeline_group_compare_start_cue>()));
	}

	timeline::internal::internal(timeline *timeline) : timeline_(timeline)
	{
	}

	auto timeline::internal::key_group_get(const uint8_t x, const uint8_t y) const
	-> std::set<std::shared_ptr<timeline_group>, timeline_group_compare_start_cue> &
	{
		return timeline_->key_group_grid_[x][y];
	}

	auto timeline::internal::group_callback_set_get() const
	-> std::set<std::shared_ptr<group_callback>, group_callback_set_compare> &
	{
		return timeline_->group_callback_set_;
	}

	string timeline::name_get() const
	{
		return name_;
	}

	void timeline::name_set(const std::string &name)
	{
		name_ = name;
	}

	bool timeline::group_add(const std::shared_ptr<timeline_group> &group)
	{
		using callback_mode = hierarchy::hierarchy_feature::callback_mode;
		const auto group_callback_ = make_shared<group_callback>();
		group_callback_->group = group;
		const auto& button_change_before_callback = [&](const auto&)
		{
			const auto& x = group->button_x.get();
			const auto& y = group->button_y.get();
			if (key_group_grid_[x][y].erase(group) == 0)
			{
				log::error("key_group_list에 group이 존재하지 않습니다: 논리적 오류");
			}
		};
		const auto& button_change_after_callback = [&](const auto&)
		{
			const auto& x = group->button_x.get();
			const auto& y = group->button_y.get();
			key_group_grid_[x][y].insert(group);
		};
		auto& button_x = group->button_x;
		auto& button_x_callback_id_list = group_callback_->button_x_callback_id_list;
		button_x_callback_id_list.reserve(2);
		button_x_callback_id_list.push_back(button_x.callback_add<callback_mode::change_before>(
			button_change_before_callback
		));
		button_x_callback_id_list.push_back(button_x.callback_add<callback_mode::change_after>(
			button_change_after_callback
		));
		auto& button_y = group->button_y;
		auto& button_y_callback_id_list = group_callback_->button_y_callback_id_list;
		button_y_callback_id_list.reserve(2);
		button_y_callback_id_list.push_back(button_y.callback_add<callback_mode::change_before>(
			button_change_before_callback
		));
		button_y_callback_id_list.push_back(button_y.callback_add<callback_mode::change_after>(
			button_change_after_callback
		));
		if (!group->start_cue)
		{
			log::error("그룹의 start_cue가 존재하지 않습니다.");
			return false;
		}
		auto& cue = group->start_cue->cue_point;
		auto& start_cue_callback_id_list = group_callback_->start_cue_callback_id_list;
		start_cue_callback_id_list.reserve(2);
		start_cue_callback_id_list.push_back(cue.callback_add<callback_mode::change_before>(
			[&](const auto&)
			{
				button_change_before_callback(0);
				if (group_callback_set_.erase(group_callback_) == 0)
				{
					log::error("group_callback_set_에 group_callback_가 존재하지 않습니다: 논리적 오류");
				}
			}
		));
		start_cue_callback_id_list.push_back(cue.callback_add<callback_mode::change_after>(
			[&](const auto&)
			{
				button_change_after_callback(0);
				group_callback_set_.insert(group_callback_);
			}
		));
		group_callback_set_.insert(group_callback_);
		const auto& x = group->button_x.get();
		const auto& y = group->button_y.get();
		key_group_grid_[x][y].insert(group);
		return true;
	}

	bool timeline::group_remove(const std::shared_ptr<timeline_group> &group)
	{
		const auto it = group_callback_set_.find<shared_ptr<timeline_group>>(group);
		if (it == group_callback_set_.end())
		{
			log::warn("그룹이 존재하지 않아 제거할 수 없습니다.");
			return false;
		}
		const auto& target = *it;
		const auto& x = target->group->button_x.get();
		const auto& y = target->group->button_y.get();
		if (key_group_grid_[x][y].erase(target->group) == 0)
		{
			log::error("key_group_list에 group이 존재하지 않습니다: 논리적 오류");
		}
		for (const auto& id : target->button_x_callback_id_list)
		{
			group->button_x.callback_remove(id);
		}
		for (const auto& id : target->button_y_callback_id_list)
		{
			group->button_y.callback_remove(id);
		}
		for (const auto& id : target->start_cue_callback_id_list)
		{
			group->start_cue->cue_point.callback_remove(id);
		}
		group_callback_set_.erase(it);
		return true;
	}

	// auto timeline::group_find_ceil(const cue_point_t &cue) -> std::shared_ptr<timeline_group>
	// {
	// 	if (cue.count() < 0)
	// 	{
	// 		log::error("음수 cue 값은 사용할 수 없습니다.");
	// 		return nullptr;
	// 	}
	// 	const auto it = group_callback_set_.lower_bound<cue_point_t>(cue);
	// 	if (it == group_callback_set_.end()) return nullptr;
	// 	return (*it)->group;
	// }

	// auto timeline::group_get(uint8_t x, uint8_t y) -> std::shared_ptr<timeline_group>
	// {
	// }

	auto timeline::last_play_group_get(uint8_t x, uint8_t y) -> std::shared_ptr<timeline_group>
	{
		return last_play_group_grid_[x][y];
	}

	void timeline::last_play_group_set(uint8_t x, uint8_t y, const std::shared_ptr<timeline_group> &group)
	{
		last_play_group_grid_[x][y] = group;
	}

	auto timeline::last_play_group_reset_all() -> void
	{
		for (auto& row : last_play_group_grid_)
		{
			for (auto& last_play_group : row)
			{
				last_play_group = nullptr;
			}
		}
	}

	timeline_page::xy::xy(const int8_t x, const int8_t y) : x(x), y(y)
	{
	}

	timeline_page::xy::xy(const uint8_t x, const uint8_t y) : x(static_cast<int8_t>(x)), y(static_cast<int8_t>(y))
	{
	}

	timeline_page::xy::xy(const int x, const int y) : x(static_cast<int8_t>(x)), y(static_cast<int8_t>(y))
	{
	}

	bool timeline_page::set_compare::operator()(const std::shared_ptr<timeline_page> &lhs,
	                                            const std::shared_ptr<timeline_page> &rhs) const
	{
		if (const auto cmp = *lhs->start_cue <=> *rhs->start_cue; cmp != 0) return cmp < 0;
		return lhs->ID_get() < rhs->ID_get();
	}

	bool timeline_page::set_compare::operator()(const std::shared_ptr<timeline_page> &lhs, const cue_point_t &rhs) const
	{
		return *lhs->start_cue->cue_point < rhs;
	}

	bool timeline_page::set_compare::operator()(const cue_point_t &lhs, const std::shared_ptr<timeline_page> &rhs) const
	{
		return lhs < *rhs->start_cue->cue_point;
	}

	timeline_page::timeline_page(const cue_point_t cue) : start_cue(timeline_cue::create(cue))
	{
	}

	bool timeline_page::next_page_set(const std::shared_ptr<timeline_page>& timeline_page, const xy xy)
	{
		// auto [it, success] = next_page_map.emplace(xy, timeline_page);
		// return success;
		next_page_map[xy] = timeline_page;
		return true;
	}

	auto timeline_page::next_page_get(const xy xy) -> std::shared_ptr<timeline_page>
	{
		const auto it = next_page_map.find(xy);
		if (it == next_page_map.end()) return nullptr;
		return it->second;
	}

	bool uniq::page_set_compare::operator()(const std::shared_ptr<page_callback> &lhs,
	                                        const std::shared_ptr<page_callback> &rhs) const
	{
		if (const auto cmp = *lhs->page->start_cue->cue_point <=> *rhs->page->start_cue->cue_point; cmp != 0)
			return cmp < 0;
		return lhs->page->ID_get() < rhs->page->ID_get();
	}

	bool uniq::page_set_compare::operator()(const std::shared_ptr<page_callback> &lhs,
		const std::shared_ptr<timeline_page> &rhs) const
	{
		return *lhs->page->start_cue->cue_point < *rhs->start_cue->cue_point;
	}

	bool uniq::page_set_compare::operator()(const std::shared_ptr<timeline_page> &lhs,
		const std::shared_ptr<page_callback> &rhs) const
	{
		return *lhs->start_cue->cue_point < *rhs->page->start_cue->cue_point;
	}

	bool uniq::page_set_compare::operator()(const std::shared_ptr<page_callback> &lhs, const cue_point_t &rhs) const
	{
		return *lhs->page->start_cue->cue_point < rhs;
	}

	bool uniq::page_set_compare::operator()(const cue_point_t &lhs, const std::shared_ptr<page_callback> &rhs) const
	{
		return lhs < *rhs->page->start_cue->cue_point;
	}

	uniq::guide_timer::guide_timer(uniq *uniq)
	{
		uniq_ = uniq;
	}

	void uniq::guide_timer::hiResTimerCallback()
	{
		uniq_->guide_cue_ += uniq_->guide_timer_interval_;
		uniq_->guide_update(true);
	}

	void uniq::guide_update(bool play_audio_flag)
	{
		if (!guide_start_) // || guide_play_)
		{
			while (!guide_group_deque_.empty())
			{
				auto guide_group = guide_group_deque_.front();
				auto x = guide_group.group->button_x.get();
				auto y = guide_group.group->button_y.get();
				launchpad_->rgb_set(x, y, 0x00, 0x00, 0x00);
				guide_group_deque_.pop_front();
			}
			return;
		}

		//guide_cue_보다 같거나 큰 첫 번째 오디오 그룹을 찾아서 guide_cue를 설정
		auto guide_target_cue = timeline::cue_point_t::max();
		auto guide_target_group = shared_ptr<timeline_group>();
		for (const auto& timeline_ : timeline_list_)
		{
			auto& group_callback_set = timeline_->internal.group_callback_set_get();
			auto it = group_callback_set.lower_bound(guide_cue_);
			if (it == group_callback_set.end()) continue;
			const auto& group = (*it)->group;
			auto cue = group->start_cue->cue_point.get();
			if (cue < guide_target_cue)
			{
				guide_target_group = group;
				guide_target_cue = cue;
			}
		}

		//가이드 페이지 확인
		auto guide_target_page = timeline_page_find_floor(guide_target_cue);
		if (!guide_target_page)
		{
			log::error("가이드 페이지가 없습니다.");
			return;
		}
		if (guide_target_page != current_page_)
		{
			int8_t page_x = 0;
			int8_t page_y = 0;
			bool page_found = false;

			// BFS로 가이드 페이지 찾기
			queue<tuple<shared_ptr<timeline_page>, int8_t, int8_t>> page_queue;
			for (const auto& [xy, page] : current_page_->next_page_map)
			{
				page_queue.emplace(page, xy.x, xy.y);
			}
			auto while_limit = current_page_->next_page_map.size() * 100;
			while (!page_queue.empty() && 0 < --while_limit)
			{
				auto page_tuple = page_queue.front();
				auto page = get<0>(page_tuple);
				auto x = get<1>(page_tuple);
				auto y = get<2>(page_tuple);
				page_queue.pop();
				if (page == guide_target_page)
				{
					page_x = x;
					page_y = y;
					page_found = true;
					break;
				}
				for (const auto& [xy_, p] : page->next_page_map)
				{
					page_queue.emplace(p, x, y);
				}
			}

			if (page_found)
			{
				if (!play_audio_flag)
				{
					//현재 페이지에 해당하는 LED 표시
					launchpad_->rgb_set(page_x, page_y, guide_color_.r, guide_color_.g, guide_color_.b);
				}
			}
			else
			{
				log::error("다음 가이드 페이지로 가는 버튼이 없습니다.");
				return;
			}
		}
		auto guide_target_next_page = page_set_.upper_bound(guide_target_page);

		while (!guide_group_deque_.empty())
		{
			auto guide_group = guide_group_deque_.front();
			if (guide_group.group->start_cue->cue_point.get() < guide_target_cue || guide_group.is_played)
			{
				if (!guide_group.is_played && play_audio_flag)
				{
					//임시
					audio_play(timeline_list_[0], guide_group.group);
					timeline_list_[0]->last_play_group_set(guide_group.group->button_x.get(), guide_group.group->button_y.get(), guide_group.group);
					//페이지 이동
					if (guide_target_page != current_page_)
					{
						current_page_ = guide_target_page;
					}
					log::info("play: " + guide_group.group->segment->source_.lock()->internal.data_get()->name_);
				}
				guide_group_deque_.pop_front();
				auto x = guide_group.group->button_x.get();
				auto y = guide_group.group->button_y.get();
				launchpad_->rgb_set(x, y, 0x00, 0x00, 0x00);
				continue;
			}
			break;
		}

		while (!guide_group_deque_.empty())
		{
			auto guide_group = guide_group_deque_.back();
			if (guide_target_cue + guide_simul_ < guide_group.group->start_cue->cue_point.get())
			{
				guide_group_deque_.pop_back();
				auto x = guide_group.group->button_x.get();
				auto y = guide_group.group->button_y.get();
				launchpad_->rgb_set(x, y, 0x00, 0x00, 0x00);
				continue;
			}
			break;
		}

		for (const auto& timeline_ : timeline_list_)
		{
			auto& group_callback_set = timeline_->internal.group_callback_set_get();
			auto it_start = group_callback_set.lower_bound(guide_target_cue);
			auto max_cue = guide_target_cue + guide_simul_;
			auto min_cue = guide_group_deque_.empty()
				? timeline::cue_point_t::min() : guide_group_deque_.back().group->start_cue->cue_point.get();
			while (it_start != group_callback_set.end())
			{
				auto cue = (*it_start)->group->start_cue->cue_point.get();
				if (max_cue < cue) break;
				if (min_cue < cue)
				{
					auto& group_callback = *it_start;
					auto& group = group_callback->group;
					log::info("group.cue: " + to_string(group->start_cue->cue_point.get().count()));
					guide_group_deque_.emplace_back(group, false);
					auto x = group->button_x.get();
					auto y = group->button_y.get();
					launchpad_->rgb_set(x, y, guide_color_.r, guide_color_.g, guide_color_.b);
				}
				++it_start;
			}
		}
	}

	void uniq::guide_togle()
	{
		if (guide_start_)
		{
			guide_stop();
		}
		else
		{
			guide_start(0us);
		}
	}

	bool uniq::guide_button_down_check(const uint8_t x, const uint8_t y)
	{
		if (guide_start_ || !guide_play_)
		{
			if (x == 3 && y == 9)
			{
				// //guide_cue_보다 작으면서 가장 큰 값의 group을 찾아서 가이드로 설정
				// shared_ptr<timeline_group> target_group;
				// for (const auto& timeline_ : timeline_list_)
				// {
				// 	auto& group_callback_set = timeline_->internal.group_callback_set_get();
				// 	auto it = group_callback_set.lower_bound(guide_cue_);
				// }
				guide_cue_ -= guide_cue_step_;
				if (guide_cue_ < 0us) guide_cue_ = 0us;
				guide_update();
				return true;
			}

			if (x == 4 && y == 9)
			{
				guide_cue_ += guide_cue_step_;
				guide_update();
				return true;
			}
		}

		//가이드 토글
		if (x == 5 && y == 9)
		{
			// guide_togle();
			guide_toggle_button_down_time_ = chrono::steady_clock::now();
			return true;
		}

		//가이드 누름 확인
		{
			auto guide_page = timeline_page_find_floor(guide_cue_);
			if (!guide_page)
			{
				log::error("가이드 페이지가 없습니다.");
				return false;
			}
			if (guide_page == current_page_)
			{
				shared_ptr<timeline_group> target_group;
				for (auto& guide_group : guide_group_deque_)
				{
					auto guide_group_x = guide_group.group->button_x.get();
					auto guide_group_y = guide_group.group->button_y.get();
					if (guide_group_x == x && guide_group_y == y && !guide_group.is_played)
					{
						guide_group.is_played = true;
						target_group = guide_group.group;
						break;
					}
				}
				if (target_group)
				{
					// guide_cue_ = target_group->start_cue->cue_point.get();
					//TODO: 가이드 누름 처리
					bool cue_update = false;
					{
						//임시
						auto key_group = timeline_list_[0]->internal.key_group_get(x, y);
						auto group_it = key_group.lower_bound(target_group);
						if (group_it == key_group.end())
						{
							timeline_list_[0]->last_play_group_set(x, y, nullptr);
						}
						else if (group_it == key_group.begin())
						{
							timeline_list_[0]->last_play_group_set(x, y, nullptr);
							cue_update = true;
						}
						else
						{
							timeline_list_[0]->last_play_group_set(x, y, *prev(group_it));
							cue_update = true;
						}
					}
					if (cue_update)
					{
						auto guide_next_cue = timeline::cue_point_t::max();
						for (const auto& timeline_ : timeline_list_)
						{
							auto& group_callback_set = timeline_->internal.group_callback_set_get();
							auto it = group_callback_set.upper_bound(guide_group_deque_.front().group->start_cue->cue_point.get());
							if (it == group_callback_set.end()) continue;
							const auto& group = (*it)->group;
							auto cue = group->start_cue->cue_point.get();
							if (cue < guide_next_cue)
							{
								guide_next_cue = cue;
							}
						}
						if (guide_next_cue != timeline::cue_point_t::max())
						{
							guide_cue_ = guide_next_cue;
						}
						else
						{
							guide_cue_ += guide_cue_step_;
						}
					}
					guide_update();
					launchpad_->rgb_set(x, y, 0x00, 0x7F, 0x00);
					// return true;
				}
			}
		}

		// //가이드 표시
		// if (guide_start_ || !guide_play_)
		// {
		// 	launchpad_->rgb_set(x, y, 0x7F, 0x00, 0x00);
		// }
		return false;
	}

	bool uniq::guide_button_up_check(uint8_t x, uint8_t y)
	{
		//가이드 토글
		if (x == 5 && y == 9)
		{
			if (chrono::steady_clock::now() - guide_toggle_button_down_time_ < guide_toggle_press_duration_)
			{
				if (guide_start_)
				{
					if (guide_play_)
					{
						guide_pause();
					}
					else
					{
						guide_resume();
					}
				}
			}
			else
			{
				if (guide_start_)
				{
					guide_stop();
				}
				else
				{
					guide_start(0us);
				}
			}
			return false;
		}
		return false;
	}

	void uniq::audio_play(const std::shared_ptr<timeline> &target_timeline, const std::shared_ptr<timeline_group> &target_group)
	{
		constexpr timeline::cue_point_t start_duration = -200ms;
		constexpr timeline::cue_point_t end_duration = 50ms;
		const auto target_group_start_cue = target_group->start_cue->cue_point.get();
		const auto& group_callback_set = target_timeline->internal.group_callback_set_get();
		//TODO: 효율적인 sync_target_add 구현
		for (auto &group_callback : group_callback_set)
		{
			if (group_callback->group == target_group) continue;
			auto& group = group_callback->group;
			auto group_end_cue = group->start_cue->cue_point.get() + group->segment->cue_length_get();
			if (group_end_cue < target_group_start_cue + start_duration) continue;
			if (target_group_start_cue + end_duration < group->start_cue->cue_point.get()) break;
			target_group->segment->sync_target_add(group_callback->group->segment->ID_get());
			// log::info("sync_target_add: " + group->segment->source_.lock()->internal.data_get()->name_);
		}
		target_group->segment->sync_duration_set(start_duration, end_duration);
		target_group->segment->time_hint_set(target_group_start_cue);
		target_group->segment->play(player_);
	}

	uniq::uniq()
	{
		current_page_ = timeline_page_create(0us);
	}

	uniq::~uniq()
	{
		launchpad_disconnect_all();
	}

	void uniq::title_set(const string &title)
	{
		title_ = title;
	}

	void uniq::producer_name_set(const string &producer_name)
	{
		producer_name_ = producer_name;
	}

	std::shared_ptr<audio_player> uniq::player_get() const
	{
		return player_;
	}

	uniq::internal::internal(uniq *uniq) : uniq_(uniq)
	{
	}

	auto uniq::internal::audio_load(unique_ptr<InputStream> input_stream, const string &extension,
	                                const string &path, const string &name) const -> shared_ptr<audio_source>
	{
		auto audio_source = audio_source::internal::audio_load(move(input_stream), extension, path, name);
		if (!audio_source)
		{
			log::warn("\"" + name + "\" 오디오 로드 실패");
			return nullptr;
		}
		uniq_->audio_source_list_.push_back(audio_source);
		return audio_source;
	}

	void uniq::audio_source_add(const std::shared_ptr<audio_source> &audio_source)
	{
		audio_source_list_.push_back(audio_source);
	}

	std::shared_ptr<timeline> uniq::timeline_create(const std::string &name)
	{
		if (name.empty())
		{
			log::warn("타임라인 이름이 비어있습니다.");
			return nullptr;
		}
		if (ranges::find_if(timeline_list_, [&name](const auto &timeline)
		{
			return timeline->name_get() == name;
		}) != timeline_list_.end())
		{
			log::warn("이미 존재하는 타임라인 이름입니다.");
			return nullptr;
		}
		auto timeline_ = timeline::create(name);
		timeline_list_.push_back(timeline_);
		return timeline_;
	}

	std::shared_ptr<timeline> uniq::timeline_get(const std::string &name)
	{
		const auto it = ranges::find_if(timeline_list_, [&name](const auto &timeline)
		{
			return timeline->name_get() == name;
		});
		if (it == timeline_list_.end())
		{
			log::warn("존재하지 않는 타임라인 이름입니다.");
			return nullptr;
		}
		return *it;
	}

	bool uniq::timeline_remove(const std::string &name)
	{
		const auto it = ranges::find_if(timeline_list_, [&name](const auto &timeline)
		{
			return timeline->name_get() == name;
		});
		if (it == timeline_list_.end())
		{
			log::warn("존재하지 않는 타임라인 이름입니다.");
			return false;
		}
		timeline_list_.erase(it);
		return true;
	}

	auto uniq::timeline_page_add(const std::shared_ptr<timeline_page> &page) -> bool
	{
		using callback_mode = hierarchy::hierarchy_feature::callback_mode;
		auto page_callback_ = make_shared<page_callback>();
		page_callback_->page = page;
		auto& cue = page->start_cue->cue_point;
		auto& callback_id_list = page_callback_->callback_id_list;
		callback_id_list.reserve(3);
		callback_id_list.push_back(cue.callback_add<callback_mode::remove_before>(
			[this, page_callback_](const auto&)
			{
				if (page_set_.erase(page_callback_) == 0)
				{
					log::error("page_set_에 page_callback_가 존재하지 않습니다: 논리적 오류");
				}
			}
		));
		callback_id_list.push_back(cue.callback_add<callback_mode::change_after>(
			[this, page_callback_](const auto&)
			{
				page_set_.insert(page_callback_);
			}
		));
		callback_id_list.push_back(cue.callback_add<callback_mode::try_remove>(
			[this, page_callback_](const auto&)
			{
				page_set_.erase(page_callback_);
			}
		));
		page_set_.insert(page_callback_);
		return true;
	}

	auto uniq::timeline_page_create(const cue_point_t &cue) -> std::shared_ptr<timeline_page>
	{
		auto timeline_page_ = timeline_page::create(cue);
		timeline_page_add(timeline_page_);
		return timeline_page_;
	}

	auto uniq::timeline_page_find_floor(const cue_point_t &cue) -> std::shared_ptr<timeline_page>
	{
		if (cue.count() < 0)
		{
			log::error("음수 cue 값은 사용할 수 없습니다.");
			return nullptr;
		}
		const auto it = page_set_.upper_bound<cue_point_t>(cue);
		if (it == page_set_.begin())
		{
			log::error("page_set_에 cue보다 작은 값이 없습니다: 논리적 오류");
			return nullptr;
		}
		return (*prev(it))->page;
	}

	auto uniq::timeline_page_remove(const shared_ptr<timeline_page> &page) -> bool
	{
		const auto it = page_set_.find<shared_ptr<timeline_page>>(page);
		if (it == page_set_.end())
		{
			log::warn("페이지가 존재하지 않아 제거할 수 없습니다.");
			return false;
		}
		page_set_.erase(it);
		return true;
	}

	auto uniq::guide_start(const cue_point_t &cue) -> void
	{
		guide_start_ = true;
		guide_play_ = false;
		auto next_page = timeline_page_find_floor(cue);
		if (!next_page)
		{
			log::error("다음 페이지가 없습니다.");
			return;
		}
		current_page_ = next_page;
		guide_cue_ = cue;
		guide_update();
	}

	auto uniq::guide_resume(const cue_point_t &cue) -> void
	{
		//TODO: guide_resume
		if (!guide_start_)
		{
			log::error("가이드가 시작되지 않았습니다.");
			return;
		}
		if (guide_play_)
		{
			log::error("가이드가 이미 재생중입니다.");
			return;
		}
		guide_play_ = true;
		//TODO: 화면에 표시된 가이드 끄기
		guide_timer_.startTimer(static_cast<int>(guide_timer_interval_.count()));
		// if (!guide_start_)
		// {
		// 	log::error("가이드가 시작되지 않았습니다.");
		// 	return;
		// }
		// guide_play_ = true;
		// auto next_page = timeline_page_find_floor(cue);
		// if (!next_page)
		// {
		// 	log::error("다음 페이지가 없습니다.");
		// 	return;
		// }
		// current_page_ = next_page;
		// guide_timer_.startTimer(guide_timer_interval_);
	}

	auto uniq::guide_pause() -> void
	{
		if (!guide_start_)
		{
			log::error("가이드가 시작되지 않았습니다.");
			return;
		}
		if (!guide_play_)
		{
			log::error("가이드가 재생중이 아닙니다.");
			return;
		}
		guide_play_ = false;
		guide_timer_.stopTimer();
		guide_update();
	}

	auto uniq::guide_stop() -> void
	{
		guide_start_ = false;
		guide_play_ = false;
		guide_update();
		// guide_timer_.stopTimer();
	}

	bool uniq::launchpad_connect(const std::shared_ptr<launchpad> &launchpad)
	{
		launchpad_ = launchpad;
		launchpad_->program_mode_set(true);
		// launchpad_callback_id_ = launchpad_->input_callback_add([this](const uint8* data, const int size)
		// {
		// 	log::info("MIDI_IN: " + String::toHexString(data, size).toStdString());
		// });
		launchpad_button_down_callback_id_ = launchpad_->input_button_down_callback_add([this](const uint8_t x, const uint8_t y, const uint8_t velocity)
		{
			pad_button_down(x, y, velocity);
		});
		launchpad_button_up_callback_id_ = launchpad_->input_button_up_callback_add([this](const uint8_t x, const uint8_t y)
		{
			pad_button_up(x, y);
		});
		return true;
	}

	bool uniq::launchpad_auto_connect()
	{
		auto midi_input_device_info_list = launchpad::get_available_input_list();
		for (auto& l : midi_input_device_info_list)
		{
			log::info(l.name.toStdString() + " => " + l.kind_name);
		}
		auto midi_output_device_info_list = launchpad::get_available_output_list();
		for (auto& l : midi_output_device_info_list)
		{
			log::info(l.name.toStdString() + " => " + l.kind_name);
		}
		if (midi_output_device_info_list.empty() || midi_input_device_info_list.empty())
		{
			log::warn("감지된 런치패드가 없습니다.");
			return false;
		}
		auto midi_output_device_info = midi_output_device_info_list.empty() ? nullptr : &midi_output_device_info_list[0];
		auto midi_input_device_info = midi_input_device_info_list.empty() ? nullptr : &midi_input_device_info_list[0];
		const auto& _launchpad = launchpad::create(player_->device_manager_get(), midi_input_device_info, midi_output_device_info);
		launchpad_connect(_launchpad);
		return true;
	}

	bool uniq::launchpad_disconnect_all()
	{
		if (!launchpad_) return false;
		launchpad_->program_mode_set(false);
		launchpad_->input_callback_remove(launchpad_callback_id_);
		launchpad_->input_button_down_callback_remove(launchpad_button_down_callback_id_);
		launchpad_->input_button_up_callback_remove(launchpad_button_up_callback_id_);
		launchpad_ = nullptr;
		return true;
	}

	void uniq::pad_button_down(const uint8_t x, const uint8_t y, const uint8_t velocity)
	{
		// log::info("pad_button_down: " + to_string(x) + ", " + to_string(y) + ", " + to_string(velocity));

		//누른키 표시
		{
			launchpad_->rgb_set(x, y, 0x00, 0x7F, 0x00);
		}

		if (guide_button_down_check(x, y)) return;

		auto target_timeline_index = -1;
		auto target_group = shared_ptr<timeline_group>();
		auto next_page_check_flag = true;
		auto cue_max = timeline::cue_point_t::max();
		auto current_page_cue = current_page_->start_cue->cue_point.get();
		auto next_page_iter = page_set_.upper_bound<timeline::cue_point_t>(current_page_cue);
		auto next_page_cue = next_page_iter == page_set_.end() ?
			timeline::cue_point_t::max() : (*next_page_iter)->page->start_cue->cue_point.get();
		for(auto i = 0; i < timeline_list_.size(); i++)
		{
			const auto& timeline_ = timeline_list_[i];
			auto& key_group = timeline_->internal.key_group_get(x, y);
			auto group_start_iter = key_group.lower_bound<timeline::cue_point_t>(current_page_cue);
			auto group_end_iter = key_group.lower_bound<timeline::cue_point_t>(next_page_cue);
			if (group_start_iter == key_group.end() || group_start_iter == group_end_iter) continue; // 재생할 그룹이 없음
			auto last_play_group = timeline_->last_play_group_get(x, y);
			auto next_page_flag = false;
			set<shared_ptr<timeline_group>, timeline::timeline_group_compare_start_cue>::iterator group_next_iter;
			if (!last_play_group)
			{
				group_next_iter = group_start_iter;
			}
			else
			{
				group_next_iter = next(key_group.find(last_play_group));
				if (group_next_iter == key_group.end() || next_page_cue <= (*group_next_iter)->start_cue->cue_point.get())
				{ // 마지막 재생 그룹이 다음 페이지로 넘어가는 경우
					group_next_iter = group_start_iter;
					next_page_flag = true;
				}
			}
			if (auto cue = (*group_next_iter)->start_cue->cue_point.get(); cue < cue_max)
			{
				target_timeline_index = i;
				target_group = *group_next_iter;
				if (!next_page_flag)
				{
					cue_max = cue;
					next_page_check_flag = false;
				}
			}
		}
		if (next_page_check_flag)
		{
			// ReSharper disable once CppTooWideScope
			const auto next_page = current_page_->next_page_get({x, y});
			if (next_page)
			{
				current_page_ = next_page;
				log::info("다음 페이지로 이동: " + to_string(current_page_->ID_get()));
				for (const auto& timeline_ : timeline_list_)
				{
					timeline_->last_play_group_reset_all();
				}
				guide_update();
				return;
			}
		}
		if (target_timeline_index == -1)
		{
			log::warn("재생할 그룹이 없습니다.");
			return;
		}
		const auto& target_timeline = timeline_list_[target_timeline_index];
		audio_play(target_timeline, target_group);
		target_timeline->last_play_group_set(x, y, target_group);
	}

	void uniq::pad_button_up(const uint8_t x, const uint8_t y)
	{
		// log::info("pad_button_up: " + to_string(x) + ", " + to_string(y));
		if (guide_button_up_check(x, y)) return;

		//누른키 표시 해제
		{
			launchpad_->rgb_set(x, y, 0x00, 0x00, 0x00);
		}
	}

	void uniq::pad_button_touch(const uint8_t x, const uint8_t y, const uint8_t velocity)
	{
		pad_button_down(x, y, velocity);
		pad_button_up(x, y);
	}
}
