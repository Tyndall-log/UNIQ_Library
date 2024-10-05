// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "api_workspace.h"

using namespace std;
using namespace uniq;
using namespace juce;

namespace uniq::workspace
{
	using namespace api;

	API id_t workspace_create()
	{
		core::workspace_preset::workspace_info.reset();
		const auto workspace = workspace::create();
		unique_lock lock(workspace_list_lock);
		workspace_list.push_back(workspace);
		core::workspace_preset::workspace_id_set.insert(workspace->ID_get());
		lock.unlock();
		return workspace->ID_get();
	}

	API bool workspace_destroy(const id_t workspace_id)
	{
		const API_raii<workspace> _workspace(workspace_id);
		if (!_workspace) return false;
		unique_lock lock(workspace_list_lock);
		const auto &it = ranges::find(workspace_list, _workspace.get_shared_ptr());
		if (it == workspace_list.end())
		{
			log::error(
				"There is no workspace with the given ID(" + to_string(workspace_id) + ") in the workspace_list");
			return false;
		}
		core::workspace_preset::workspace_id_set.erase(workspace_id);
		workspace_list.erase(it);
		lock.unlock();
		return true;
	}

#pragma region workspace
	id_t workspace_project_create(const id_t workspace_id)
	{
		const API_raii<workspace> _workspace(workspace_id);
		if (!_workspace) return 0;
		return _workspace->uniq_create()->ID_get();
	}

	bool workspace_project_add(const id_t workspace_id, const id_t project_id)
	{
		const API_raii<workspace> _workspace(workspace_id);
		const API_raii<project::project> _project(project_id);
		if (!_workspace || !_project) return false;
		return _workspace->uniq_add(_project.get_shared_ptr());
	}

	bool workspace_project_remove(const id_t workspace_id, const id_t project_id)
	{
		const API_raii<workspace> _workspace(workspace_id);
		const API_raii<project::project> _project(project_id);
		if (!_workspace || !_project) return false;
		return _workspace->uniq_remove(_project.get_shared_ptr());
	}

	id_t *workspace_project_list_get(const id_t workspace_id, size_t *size)
	{
		const API_raii<workspace> _workspace(workspace_id);
		if (!_workspace) return nullptr;
		const auto uniq_list = _workspace->uniq_list_get();
		*size = uniq_list.size();
		auto *list = new id_t[*size];
		for (size_t i = 0; i < *size; i++)
		{
			list[i] = uniq_list[i]->ID_get();
		}
		return list;
	}

	void workspace_name_set(const id_t workspace_id, const char *name)
	{
		const API_raii<workspace> _workspace(workspace_id);
		if (!_workspace) return;
		_workspace->name_set(name);
	}

	char *workspace_name_get(const id_t workspace_id)
	{
		const API_raii<workspace> _workspace(workspace_id);
		if (!_workspace) return nullptr;
		return strdup(_workspace->name_get().c_str());
	}
#pragma endregion workspace
}
