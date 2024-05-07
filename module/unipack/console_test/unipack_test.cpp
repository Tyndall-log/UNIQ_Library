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
	vector<string> files;
	for (const auto &entry : filesystem::directory_iterator(current_path))
	{
		if (entry.is_regular_file())
		{
			const auto& path = entry.path();
			if (auto ext = path.extension().string(); extensions.contains(ext))
			{
				files.push_back(path.string());
			}
		}
	}
	ranges::sort(files);
	cout << "찾은 유니팩: " << endl;
	for (const auto &f : files)
	{
		cout << "\t" << f << endl;
	}
	if (!files.empty())
	{
		file = files.front();
	}

	if (file.empty() || !filesystem::exists(file))
	{
		cout << "현재 폴더 위치에서 *.zip 또는 *.uni 파일을 찾을 수 없습니다." << endl;
		cout << "종료하려면 아무 키나 누르세요.";
		cin.get();
		return 1;
	}

	const auto uniq = unipack::unipack::load(file);
	uniq->launchpad_auto_connect();

	cout << "종료하려면 아무 키나 누르세요.";
	cin.get();
	return 0;
}
