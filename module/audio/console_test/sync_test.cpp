// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "main.h"

using namespace std;
using namespace juce;
using namespace uniq;
using namespace uniq::audio;

int sync_test1()
{
	cout << "sync_test1 시작" << endl;
	int a;
	auto player = audio_player::create();
	auto as = audio_source::audio_load(audio_file_path.file2);
	cin >> a;
	as->play(player);
	cin >> a;
	cout << "sync_test1 끝" << endl;
	// uniq::internal::audio_format_manager::format_manager_ = nullptr;
	return 0;
}

int sync_test()
{
	cout << "sync_test 시작" << endl;
	sync_test1();
	return 0;
}
