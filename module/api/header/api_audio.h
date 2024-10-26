// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "api.h"

namespace uniq::audio
{
	using namespace api;

#pragma region audio_source
#pragma endregion audio_source

#pragma region audio_segment
	API bool play(id_t segment_id, id_t player_id);
	API uint64_t cue_length_get(id_t segment_id);
	API bool sync_target_add(id_t segment_id, id_t target_id);
	API bool sync_target_remove(id_t segment_id, id_t target_id);
	API bool sync_target_remove_all(id_t segment_id);
	API bool fade_out_end_time_set(id_t segment_id, uint64_t time);
	API bool fade_out_target_add(id_t segment_id, id_t target_id);
	API bool fade_out_target_add_segment(id_t segment_id, id_t target_id);
	API bool time_hint_set(id_t segment_id, uint64_t time_hint);
	API bool sync_duration_set(id_t segment_id, uint64_t start, uint64_t end);
	API void start_cue_change(id_t segment_id, id_t cue_id);
	API void end_cue_change(id_t segment_id, id_t cue_id);
	API float* waveform_get(id_t segment_id, uint64_t window_size, uint8_t channel);
	API float* waveform_get_part(id_t segment_id, int64_t start, int64_t end, uint64_t window_size, uint8_t channel);
#pragma endregion audio_segment
}