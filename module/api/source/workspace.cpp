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
		const auto& it = ranges::find(workspace_list, _workspace.get_shared_ptr());
		if (it == workspace_list.end())
		{
			log::error("There is no workspace with the given ID(" + to_string(workspace_id) + ") in the workspace_list");
			return false;
		}
		core::workspace_preset::workspace_id_set.erase(workspace_id);
		workspace_list.erase(it);
		lock.unlock();
		return true;
	}
}