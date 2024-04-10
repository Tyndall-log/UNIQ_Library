// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "main.h"

using namespace std;
using namespace juce;
using namespace uniq;

int workspace_test1()
{
	cout << "workspace_test1" << endl;
	::uniq::uniq::create();
	int a;
	auto player = audio_player::create();
	// auto as = audio_source::audio_load(audio_file_path.file2);
	cin >> a;
	// as->play(player);
	cin >> a;
	cout << "workspace_test1 끝" << endl;
	return 0;
}
