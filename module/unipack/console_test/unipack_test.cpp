// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include <filesystem>

#include "main.h"
#include "secret.h"

using namespace std;
using namespace juce;
using namespace uniq;

int unipack_test1()
{
	cout << "unipack_test1" << endl;

	const auto current_path = filesystem::current_path();
	const set<string> extensions = {".zip", ".uni"};
	string file = unipack_file_path::file1;
	for (const auto &entry : filesystem::directory_iterator(current_path))
	{
		if (entry.is_regular_file())
		{
			const auto& path = entry.path();
			if (auto ext = path.extension().string(); extensions.contains(ext))
			{
				file = path.string();
				break;
			}
		}
	}

	if (file.empty())
	{
		cout << "파일이 없습니다." << endl;
		return 1;
	}

	const auto uniq = unipack::unipack::load(file);
	uniq->launchpad_auto_connect();

	cout << "종료하려면 아무 키나 누르세요.";
	cin.get();
	return 0;
}
