// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "system.h"
#include "timeline_cuepoint.h"

namespace uniq
{
	class timeline : public ID<timeline>
	{
		template<typename T> friend class ID;
		using cuetime_t = int64_t; //1ns (10^-9s) -> max 292 years
	private:
		timeline() = default;
		std::map<cuetime_t, std::shared_ptr<timeline_cuepoint>> cuepoints;
	public:
		static std::shared_ptr<timeline> create()
		{
			return ID<timeline>::create();
		}
		void add_cuepoint(cuetime_t cuetime, std::shared_ptr<timeline_cuepoint> cuepoint)
		{
			
			//cuepoint->remove_all_parent
			//cuepoint->add_parent
			cuepoints.emplace(cuetime, cuepoint);
		}
	};
}