// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "workspace_preset.h"

using namespace std;
using namespace uniq;

namespace uniq::core::workspace_preset
{
	void workspace_info_class::set(const id_t id, const source_location location)
	{
		this->id = id;
		api_function_name = location.function_name();
	}

	void workspace_info_class::reset()
	{
		id = 0;
		api_function_name = default_name;
	}

	id_t workspace_info_class::get_id() const
	{
		return id;
	}

	string_view workspace_info_class::get_function_name() const
	{
		return api_function_name;
	}

	thread_local workspace_info_class workspace_info;
	unordered_set<id_t> workspace_id_set;
}