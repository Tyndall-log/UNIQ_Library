// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "api_audio.h"

namespace uniq::audio
{
	using namespace api;

#pragma region audio_source
#pragma endregion audio_source

#pragma region audio_segment
	API bool play(const id_t segment_id, const id_t player_id)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return false;
		const auto _player = core::ID_manager::get_shared_ptr<audio_player>(player_id);
		return _segment->play(_player);
	}

	uint64_t cue_length_get(const id_t segment_id)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return 0;
		return _segment->cue_length_get().count();
	}

	bool sync_target_add(const id_t segment_id, const id_t target_id)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return false;
		const auto _target = core::ID_manager::get_shared_ptr<audio_segment>(target_id);
		return _segment->sync_target_add(_target);
	}

	bool sync_target_remove(const id_t segment_id, const id_t target_id)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return false;
		const auto _target = core::ID_manager::get_shared_ptr<audio_segment>(target_id);
		return _segment->sync_target_remove(_target);
	}

	bool sync_target_remove_all(const id_t segment_id)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return false;
		return _segment->sync_target_remove_all();
	}

	bool fade_out_end_time_set(const id_t segment_id, const uint64_t time)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return false;
		return _segment->fade_out_end_time_set(std::chrono::microseconds(time));
	}

	bool fade_out_target_add(const id_t segment_id, const id_t target_id)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return false;
		const auto _target = core::ID_manager::get_shared_ptr<audio_segment>(target_id);
		return _segment->fade_out_target_add(_target);
	}

	bool fade_out_target_add_segment(const id_t segment_id, const id_t target_id)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return false;
		const auto _target = core::ID_manager::get_shared_ptr<audio_segment>(target_id);
		return _segment->fade_out_target_add(_target);
	}

	bool time_hint_set(const id_t segment_id, const uint64_t time_hint)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return false;
		return _segment->time_hint_set(std::chrono::microseconds(time_hint));
	}

	bool sync_duration_set(const id_t segment_id, const uint64_t start, const uint64_t end)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return false;
		return _segment->sync_duration_set(std::chrono::microseconds(start), std::chrono::microseconds(end));
	}

	void start_cue_change(const id_t segment_id, const id_t cue_id)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return;
		const auto _cue = core::ID_manager::get_shared_ptr<audio_cue>(cue_id);
		_segment->start_cue_change(_cue);
	}

	void end_cue_change(const id_t segment_id, const id_t cue_id)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return;
		const auto _cue = core::ID_manager::get_shared_ptr<audio_cue>(cue_id);
		_segment->end_cue_change(_cue);
	}

	float *waveform_get(const id_t segment_id, const uint64_t window_size, const uint8_t channel)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return nullptr;
		return _segment->waveform_get(window_size, channel);
	}

	float *waveform_get_part(const id_t segment_id, const int64_t start, const int64_t end, const uint64_t window_size, const uint8_t channel)
	{
		const API_raii<audio_segment> _segment(segment_id);
		if (!_segment) return nullptr;
		return _segment->waveform_get_part(start, end, window_size, channel);
	}
#pragma endregion audio_segment
}
