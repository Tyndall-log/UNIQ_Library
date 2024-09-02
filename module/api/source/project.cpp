// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "api.h"

namespace uniq::project
{
	using namespace api;

	API void title_set(const id_t project_id, const char* title)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		_project->title_set(title);
	}

	API void producer_name_set(const id_t project_id, const char* producer_name)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return;
		_project->producer_name_set(producer_name);
	}

	API id_t timeline_create(const id_t project_id, const char* name)
	{
		const API_raii<project> _project(project_id);
		if (!_project) return 0;
		const auto& t = _project->timeline_create(name);
		if (!t) return 0;
		return t->ID_get();
	}
}