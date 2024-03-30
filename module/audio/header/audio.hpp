// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

namespace uniq
{
	template<audio_source::cue_add_mode _cue_add_mode>
	bool audio_source::cue_add(std::uint64_t cue)
	{
		if (!data_) //오디오 데이터가 없으면 실패
			return false;
		std::shared_lock lock(data_->mutex_);
		if (data_->buffer_.getNumSamples() <= cue) //큐가 오디오 데이터의 길이보다 길면 실패
			return false;
		auto cue_it = cue_point_list_.lower_bound(cue); //cue가 같거나 큰 첫번째 큐를 찾음
		// cue_it != cue_point_list_.end()임이 보장됨
		if (**cue_it <=> cue == 0) //이미 위치가 같은 큐가 있으면 실패
			return false;

		//새로운 큐를 추가
		auto new_cue = audio_cue::create(cue);
		cue_it = cue_point_list_.insert(cue_it, new_cue);

		//세그먼트 분할 여부 확인
		static_assert(_cue_add_mode != cue_add_mode::segment_split_not_allowed, "segment_split_not_allowed is not allowed in cue_add");
		auto segment_it = segment_start_set_.lower_bound(static_cast<uint64_t>(**std::prev(cue_it)));
		if constexpr (_cue_add_mode == cue_add_mode::segment_split_keep_front)
		{
			for (auto it = segment_it; it != segment_start_set_.end() && *(*it)->start_cue_ < cue; ++it)
			{
				(*it)->start_cue_change(new_cue);
			}
		}
		if constexpr (_cue_add_mode == cue_add_mode::segment_split_keep_back)
		{
			for (auto it = segment_it; it != segment_start_set_.end() && *(*it)->start_cue_ < cue; ++it)
			{
				(*it)->end_cue_change(new_cue);
			}
		}
		return true;
	}

	template<audio_source::cue_remove_mode _cue_remove_mode>
	bool audio_source::cue_remove(std::uint64_t cue)
	{
		if (!data_) //오디오 데이터가 없으면 실패
			return false;
		std::shared_lock lock(data_->mutex_);
		if (data_->buffer_.getNumSamples() <= cue) //큐가 오디오 데이터의 길이보다 길면 실패
			return false;
		auto cue_it = cue_point_list_.lower_bound(cue); //cue가 같거나 큰 첫번째 큐를 찾음
		if (**cue_it <=> cue != 0) //위치가 같은 큐가 없으면 실패
			return false;

		//세그먼트 병합 여부 확인
		static_assert(_cue_remove_mode != cue_remove_mode::segment_merge_not_allowed, "segment_merge_not_allowed is not allowed in cue_remove");
		auto segment_it = segment_start_set_.lower_bound(static_cast<uint64_t>(**std::prev(cue_it)));
		if constexpr (_cue_remove_mode == cue_remove_mode::segment_merge_remove_front)
		{
			for (auto it = segment_it; it != segment_start_set_.end() && *(*it)->start_cue_ < cue; ++it)
			{
				(*it)->start_cue_change(*std::next(cue_it));
			}
		}
		if constexpr (_cue_remove_mode == cue_remove_mode::segment_merge_remove_back)
		{
			for (auto it = segment_it; it != segment_start_set_.end() && *(*it)->start_cue_ < cue; ++it)
			{
				(*it)->end_cue_change(*std::next(cue_it));
			}
		}
		return true;
	}
}
