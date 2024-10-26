// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "api.h"

namespace uniq::project
{
	using namespace api;

#pragma region project
	API void *title_get(id_t project_id);
	API void title_set(id_t project_id, const char *title);
	API void *producer_name_get(id_t project_id);
	API void producer_name_set(id_t project_id, const char *producer_name);
	// API void* player_get(id_t project_id);
	API id_t audio_load(id_t project_id, const char *path);
	API void audio_source_add(id_t project_id, id_t audio_source_id);
	API id_t timeline_create(id_t project_id, const char *name);
	API id_t timeline_get(id_t project_id, const char *name);
	API bool timeline_remove(id_t project_id, const char *name);
	API bool timeline_page_add(id_t project_id, id_t page_id);
	API id_t timeline_page_create(id_t project_id, int64_t cue);
	API id_t timeline_page_find_floor(id_t project_id, int64_t cue);
	API bool timeline_page_remove(id_t project_id, id_t page_id);
	API void guide_start(id_t project_id, int64_t cue);
	API void guide_resume(id_t project_id, int64_t cue);
	API void guide_position_set(id_t project_id, int64_t cue);
	API int64_t guide_position_get(id_t project_id);
	API void guide_pause(id_t project_id);
	API void guide_stop(id_t project_id);
	API bool launchpad_connect(id_t project_id, id_t launchpad_id);
	API bool launchpad_auto_connect(id_t project_id);
	API bool launchpad_disconnect_all(id_t project_id);
	API void pad_button_down(id_t project_id, uint8_t x, uint8_t y, uint8_t velocity);
	API void pad_button_up(id_t project_id, uint8_t x, uint8_t y);
	API void pad_button_touch(id_t project_id, uint8_t x, uint8_t y, uint8_t velocity);
#pragma endregion project

#pragma region timeline
	API void *timeline_name_get(id_t timeline_id);
	API void timeline_name_set(id_t timeline_id, const char *name);
	API bool timeline_group_add(id_t timeline_id, id_t group_id);
	API bool timeline_group_remove(id_t timeline_id, id_t group_id);
#pragma endregion timeline

#pragma region timeline_cue
	API void cue_point_set(id_t timeline_id, int64_t cue);
	API int64_t cue_point_get(id_t timeline_id);
#pragma endregion timeline_cue

#pragma region timeline_group
	API int8_t group_button_x_get(id_t group_id);
	API void group_button_x_set(id_t group_id, int8_t x);
	API int8_t group_button_y_get(id_t group_id);
	API void group_button_y_set(id_t group_id, int8_t y);
	API void group_press_duration_set(id_t group_id, int64_t duration);
	API int64_t group_press_duration_get(id_t group_id);
	API void group_segment_set(id_t group_id, id_t segment_id);
	API id_t group_segment_get(id_t group_id);
	API void group_start_cue_set(id_t group_id, id_t start_cue_id);
	API id_t group_start_cue_get(id_t group_id);
	API void group_lightshow_data_set(id_t group_id, id_t lightshow_data_id);
	API id_t group_lightshow_data_get(id_t group_id);
#pragma endregion timeline_group
}
