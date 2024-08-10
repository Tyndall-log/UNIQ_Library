// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "main.h"

using namespace std;
using namespace juce;
using namespace uniq;

int workspace_test1()
{
	cout << "workspace_test1" << endl;
	auto project = ::uniq::uniq::create();
	int a;
	auto player = audio_player::create();
	// auto as = audio_source::audio_load(audio_file_path.file2);
	cin >> a;
	// as->play(player);
	cin >> a;
	cout << "workspace_test1 끝" << endl;
	static auto offset =
			reinterpret_cast<uintptr_t>(&reinterpret_cast<core::api::API_callback_message_base*>(0)->api_workspace_id);
	cout << "offset: " << offset << endl;
	return 0;
}
