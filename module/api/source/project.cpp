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
		const auto _launchpad = core::ID_manager::get_shared_ptr<launchpad>(launchpad_id);
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
}
