// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "core.h"
#include "audio.h"
#include "launchpad.h"
#include "uniq.h"
#include "unipack.h"

namespace uniq::workspace
{
	class workspace : public ID<workspace>
	{
		std::string name;
		std::vector<std::shared_ptr<uniq>> uniq_list;
	public:

		void uniq_add(const std::shared_ptr<uniq>& uniq);
		bool uniq_remove(const std::shared_ptr<uniq>& uniq);
		[[nodiscard]] std::vector<std::shared_ptr<uniq>> uniq_list_get() const;
		void name_set(const std::string& name);
		[[nodiscard]] std::string name_get() const;
	};
}

extern std::vector<std::shared_ptr<uniq::workspace::workspace>> workspace_list;
extern uniq::spin_lock workspace_list_lock;
