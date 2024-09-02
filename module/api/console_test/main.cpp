// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "api.h"
#include "secret.h"
#include "api_workspace.h"
#include "api_unipack.h"

// #include "../source/workspace.cpp"
// #include "../source/unipack.cpp"

using namespace std;
using namespace uniq;

int main()
{
	auto id = workspace::workspace_create();
	auto project_id = unipack::unipack_load(id, test_path.c_str());
	log::info("Project ID: " + to_string(project_id));
	int a;
	cin >> a;
	workspace::workspace_destroy(id);
	return 0;
}