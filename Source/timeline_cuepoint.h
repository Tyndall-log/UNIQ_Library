// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "system.h"

namespace uniq
{
	class timeline_cuepoint : public ID<timeline_cuepoint>
	{
		template<typename T> friend class ID;
	private:
		timeline_cuepoint() = default;
	public:
		static std::shared_ptr<timeline_cuepoint> create()
		{
			return ID<timeline_cuepoint>::create();
		}
	};
}