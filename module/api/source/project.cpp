// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "api_project.h"

namespace uniq::project
{
	using namespace api;

	using cue_point_t = timeline_cue::cue_point_t;

#pragma region project
	API void *title_get(const id_t project_id)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return nullptr;
		return strdup(_project->title_get().c_str());
	}

	API void title_set(const id_t project_id, const char *title)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		_project->title_set(title);
	}

	void *producer_name_get(const id_t project_id)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return nullptr;
		return strdup(_project->producer_name_get().c_str());
	}

	API void producer_name_set(const id_t project_id, const char *producer_name)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		_project->producer_name_set(producer_name);
	}

	id_t audio_load(const id_t project_id, const char *path)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return false;
		return _project->audio_load(path)->ID_get();
	}

	void audio_source_add(const id_t project_id, const id_t audio_source_id)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		const auto _audio_source = core::ID_manager::get_shared_ptr<audio_source>(audio_source_id);
		if (!_audio_source) return;
		_project->audio_source_add(_audio_source);
	}

	API id_t timeline_create(const id_t project_id, const char *name)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return 0;
		const auto &t = _project->timeline_create(name);
		if (!t) return 0;
		return t->ID_get();
	}

	API id_t timeline_get(const id_t project_id, const char *name)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return 0;
		const auto &t = _project->timeline_get(name);
		if (!t) return 0;
		return t->ID_get();
	}

	API bool timeline_remove(const id_t project_id, const char *name)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return false;
		return _project->timeline_remove(name);
	}

	API bool timeline_page_add(const id_t project_id, const id_t page_id)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return false;
		const auto _page = core::ID_manager::get_shared_ptr<timeline_page>(page_id);
		if (!_page) return false;
		return _project->timeline_page_add(_page);
	}

	API id_t timeline_page_create(const id_t project_id, const int64_t cue)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return 0;
		const auto &t = _project->timeline_page_create(cue_point_t(cue));
		if (!t) return 0;
		return t->ID_get();
	}

	API id_t timeline_page_find_floor(const id_t project_id, const int64_t cue)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return 0;
		const auto &t = _project->timeline_page_find_floor(cue_point_t(cue));
		if (!t) return 0;
		return t->ID_get();
	}

	API bool timeline_page_remove(const id_t project_id, const id_t page_id)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return false;
		const auto _page = core::ID_manager::get_shared_ptr<timeline_page>(page_id);
		if (!_page) return false;
		return _project->timeline_page_remove(_page);
	}

	API void guide_start(const id_t project_id, const int64_t cue)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		_project->guide_start(cue_point_t(cue));
	}

	API void guide_resume(const id_t project_id, const int64_t cue)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		_project->guide_resume(cue_point_t(cue));
	}

	API void guide_position_set(const id_t project_id, const int64_t cue)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		_project->guide_position_set(cue_point_t(cue));
	}

	API int64_t guide_position_get(const id_t project_id)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return 0;
		return _project->guide_position_get().count();
	}

	API void guide_pause(const id_t project_id)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		_project->guide_pause();
	}

	API void guide_stop(const id_t project_id)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		_project->guide_stop();
	}

	API bool launchpad_connect(const id_t project_id, const id_t launchpad_id)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return false;
		const auto _launchpad = core::ID_manager::get_shared_ptr<launchpad::launchpad>(launchpad_id);
		if (!_launchpad) return false;
		return _project->launchpad_connect(_launchpad);
	}

	API bool launchpad_auto_connect(const id_t project_id)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return false;
		return _project->launchpad_auto_connect();
	}

	API bool launchpad_disconnect_all(const id_t project_id)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return false;
		return _project->launchpad_disconnect_all();
	}

	API void pad_button_down(const id_t project_id, const uint8_t x, const uint8_t y, const uint8_t velocity)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		_project->pad_button_down(x, y, velocity);
	}

	API void pad_button_up(const id_t project_id, const uint8_t x, const uint8_t y)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		_project->pad_button_up(x, y);
	}

	API void pad_button_touch(const id_t project_id, const uint8_t x, const uint8_t y, const uint8_t velocity)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		_project->pad_button_touch(x, y, velocity);
	}
#pragma endregion project

#pragma region timeline
	void * timeline_name_get(const id_t timeline_id)
	{
		const API_raii<timeline> _timeline(timeline_id);
		if (!_timeline) return nullptr;
		return strdup(_timeline->name_get().c_str());
	}

	API void timeline_name_set(const id_t timeline_id, const char *name)
	{
		const API_raii<timeline> _timeline(timeline_id);
		if (!_timeline) return;
		_timeline->name_set(name);
	}

	API bool timeline_group_add(const id_t timeline_id, const id_t group_id)
	{
		const API_raii<timeline> _timeline(timeline_id);
		if (!_timeline) return false;
		const auto _group = core::ID_manager::get_shared_ptr<timeline_group>(group_id);
		if (!_group) return false;
		return _timeline->group_add(_group);
	}

	API bool timeline_group_remove(const id_t timeline_id, const id_t group_id)
	{
		const API_raii<timeline> _timeline(timeline_id);
		if (!_timeline) return false;
		const auto _group = core::ID_manager::get_shared_ptr<timeline_group>(group_id);
		if (!_group) return false;
		return _timeline->group_remove(_group);
	}
#pragma endregion timeline

#pragma region timeline_cue
	void cue_point_set(const id_t timeline_id, int64_t cue)
	{
		const API_raii<timeline_cue> _timeline(timeline_id);
		if (!_timeline) return;
		_timeline->cue_point_set(cue_point_t(cue));
	}

	int64_t cue_point_get(const id_t timeline_id)
	{
		const API_raii<timeline_cue> _timeline(timeline_id);
		if (!_timeline) return 0;
		return _timeline->cue_point_get().count();
	}
#pragma endregion timeline_cue

#pragma region timeline_group
	int8_t group_button_x_get(id_t group_id)
	{
		const API_raii<timeline_group> _group(group_id);
		if (!_group) return 0;
		return _group->button_x_get();
	}

	void group_button_x_set(id_t group_id, int8_t x)
	{
		const API_raii<timeline_group> _group(group_id);
		if (!_group) return;
		_group->button_x_set(x);
	}

	int8_t group_button_y_get(id_t group_id)
	{
		const API_raii<timeline_group> _group(group_id);
		if (!_group) return 0;
		return _group->button_y_get();
	}

	void group_button_y_set(id_t group_id, int8_t y)
	{
		const API_raii<timeline_group> _group(group_id);
		if (!_group) return;
		_group->button_y_set(y);
	}

	void group_press_duration_set(id_t group_id, int64_t duration)
	{
		const API_raii<timeline_group> _group(group_id);
		if (!_group) return;
		_group->press_duration_set(timeline_group::press_duration_t(duration));
	}

	int64_t group_press_duration_get(id_t group_id)
	{
		const API_raii<timeline_group> _group(group_id);
		if (!_group) return 0;
		return _group->press_duration_get().count();
	}

	void group_segment_set(id_t group_id, id_t segment_id)
	{
		const API_raii<timeline_group> _group(group_id);
		if (!_group) return;
		const auto _segment = core::ID_manager::get_shared_ptr<audio_segment>(segment_id);
		if (!_segment) return;
		_group->segment_set(_segment);
	}

	id_t group_segment_get(id_t group_id)
	{
		const API_raii<timeline_group> _group(group_id);
		if (!_group) return 0;
		const auto _segment = _group->segment_get();
		if (!_segment) return 0;
		return _segment->ID_get();
	}

	void group_start_cue_set(id_t group_id, id_t start_cue_id)
	{
		const API_raii<timeline_group> _group(group_id);
		if (!_group) return;
		const auto _start_cue = core::ID_manager::get_shared_ptr<timeline_cue>(start_cue_id);
		if (!_start_cue) return;
		_group->start_cue_set(_start_cue);
	}

	id_t group_start_cue_get(id_t group_id)
	{
		const API_raii<timeline_group> _group(group_id);
		if (!_group) return 0;
		const auto _start_cue = _group->start_cue_get();
		if (!_start_cue) return 0;
		return _start_cue->ID_get();
	}

	void group_lightshow_data_set(id_t group_id, id_t lightshow_data_id)
	{
		const API_raii<timeline_group> _group(group_id);
		if (!_group) return;
		const auto _lightshow_data = core::ID_manager::get_shared_ptr<lightshow::lightshow_data>(lightshow_data_id);
		if (!_lightshow_data) return;
		_group->lightshow_data_set(_lightshow_data);
	}

	id_t group_lightshow_data_get(id_t group_id)
	{
		const API_raii<timeline_group> _group(group_id);
		if (!_group) return 0;
		const auto _lightshow_data = _group->lightshow_data_get();
		if (!_lightshow_data) return 0;
		return _lightshow_data->ID_get();
	}
#pragma endregion timeline_group
}
