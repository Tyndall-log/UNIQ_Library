// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "main.h"

using namespace std;
using namespace uniq;

int api_test1()
{
	cout << "api_test1" << endl;
	auto id = workspace::workspace_create();
	auto project_id = unipack::unipack_load(id, test_path.c_str());
	log::info("Project ID: " + to_string(project_id));
	int a;
	cin >> a;
	workspace::workspace_destroy(id);
	return 0;
}


