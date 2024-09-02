// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "api.h"

namespace uniq::unipack
{
	API id_t unipack_load(id_t workspace_id, const char* zip_path);
}
