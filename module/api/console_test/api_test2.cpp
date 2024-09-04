// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "main.h"

using namespace std;
using namespace uniq;

int api_test2()
{
	cout << "api_test1" << endl;
	const auto workspace_id = workspace::workspace_create();
	const auto project_id = unipack::unipack_load(workspace_id, test_path.c_str());
	log::info("Project ID: " + to_string(project_id));
	project::launchpad_auto_connect(project_id);
	cin.get();
	workspace::workspace_destroy(workspace_id);
	return 0;
}