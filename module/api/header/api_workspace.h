// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "api.h"

namespace uniq::workspace
{
	API id_t workspace_create();
	API bool workspace_destroy(id_t workspace_id);

#pragma region workspace
	API id_t workspace_project_create(id_t workspace_id);
	API bool workspace_project_add(id_t workspace_id, id_t project_id);
	API bool workspace_project_remove(id_t workspace_id, id_t project_id);
	API id_t* workspace_project_list_get(id_t workspace_id, size_t *size);
	API void workspace_name_set(id_t workspace_id, const char* name);
	API char* workspace_name_get(id_t workspace_id);
#pragma endregion workspace
}