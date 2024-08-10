// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "workspace.h"

using namespace std;
using namespace uniq;
using namespace juce;

vector<shared_ptr<workspace::workspace>> workspace_list;
spin_lock workspace_list_lock;

namespace uniq::workspace
{
	void workspace::uniq_add(const std::shared_ptr<uniq> &uniq)
	{
		uniq_list.push_back(uniq);
	}

	bool workspace::uniq_remove(const std::shared_ptr<uniq> &uniq)
	{
		const auto it = std::ranges::find(uniq_list, uniq);
		if (it == uniq_list.end()) return false;
		uniq_list.erase(it);
		return true;
	}

	std::vector<std::shared_ptr<uniq>> workspace::uniq_list_get() const
	{
		return uniq_list;
	}

	void workspace::name_set(const std::string &name)
	{
		this->name = name;
		core::api::callback_manager.RAC(ID_get(), name);
	}

	std::string workspace::name_get() const
	{
		return name;
	}
}

