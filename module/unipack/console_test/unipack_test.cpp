// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "main.h"
#include "secret.h"

using namespace std;
using namespace juce;
using namespace uniq;

int unipack_test1()
{
	cout << "unipack_test1" << endl;

	// cin.get();
	auto uniq = unipack::unipack::load(unipack_file_path::file1);
	uniq->launchpad_auto_connect();

	cout << "종료하려면 아무 키나 누르세요.";
	cin.get();
	return 0;
}
