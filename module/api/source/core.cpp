// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "api_core.h"

namespace uniq::core
{
	API id_t workspace_ID_get(const id_t obj_id)
	{
		return ID_manager::get_workspace_ID(obj_id);
	}
}