// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#ifdef UNIQ_DLL_API

#include "workspace.h"

using namespace std;
using namespace uniq;
// using namespace core::api;
using namespace juce;

API ::uniq::id_t workspace_create()
{
	core::workspace_preset::workspace_info.reset();
	const auto workspace = workspace::workspace::create();
	unique_lock lock(workspace_list_lock);
	workspace_list.push_back(workspace);
	core::workspace_preset::workspace_id_set.insert(workspace->ID_get());
	lock.unlock();
	return workspace->ID_get();
}

API bool workspace_delete(const ::uniq::id_t workspace_id)
{
	if (workspace_id == 0)
	{
		log::error("workspace_id is 0");
		return false;
	}
	core::api::workspace_info_raii w(workspace_id);
	const auto workspace_o = ID_manager::get_shared_ptr_by_ID<workspace::workspace>(workspace_id);
	if (!workspace_o)
	{
		log::error("There is no workspace with the given ID(" + to_string(workspace_id) + ")");
		return false;
	}
	const auto& workspace = workspace_o.value();
	unique_lock lock(workspace_list_lock);
	const auto& it = ranges::find(workspace_list, workspace);
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

API ::uniq::id_t unipack_load(const ::uniq::id_t workspace_id, const string& zip_path)
{
	if (workspace_id == 0)
	{
		log::error("workspace_id is 0");
		return 0;
	}
	core::api::workspace_info_raii w(workspace_id);
	const auto workspace_o = ID_manager::get_shared_ptr_by_ID<workspace::workspace>(workspace_id);
	if (!workspace_o)
	{
		log::error("There is no workspace with the given ID(" + to_string(workspace_id) + ")");
		return 0;
	}
	const auto& workspace = workspace_o.value();
	const auto unipack = unipack::unipack::load(zip_path);
	if (!unipack)
	{
		log::error("unipack is nullptr");
		return 0;
	}
	workspace->uniq_add(unipack);
	return unipack->ID_get();
}


#endif
