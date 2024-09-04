// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "main.h"

#include <random>

using namespace std;
using namespace juce;
using namespace uniq;

int launchpad_test1()
{
	cout << "launchpad_test1" << endl;

	auto adm = audio_device_manager::get();
	auto lm = launchpad_manager::instance_get();
	auto lpl = lm->launchpad_list_get();
	cout << "연결할 런치패드 목록:" << endl;
	for (auto i = 0; i < lpl.size(); i++)
	{
		cout << "\t[" + to_string(i) + "] : " + lpl[i]->input_kind_name_get() << endl;
	}
	if (lpl.empty())
	{
		cout << "연결할 런치패드가 없습니다." << endl;
		return -1;
	}
	int index;
	if (lpl.size() == 1)
	{
		index = 0;
		cout << "런치패드가 하나만 연결되어 있으므로, 자동으로 선택됩니다." << endl;
	}
	else
	{
		cout << "연결할 런치패드를 선택하세요: ";
		while (true)
		{
			cin >> index;
			if (cin.fail() || index < 0 || index >= lpl.size())
			{
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				cout << "잘못된 입력입니다. 다시 입력하세요: ";
			}
			else
			{
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				break;
			}
		}
	}
	const auto lp = lpl[index];
	cout << "선택된 런치패드: " << lp->input_kind_name_get() << endl;
	lp->program_mode_set();

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution dis(0, 255);
	const auto ibdc = lp->input_button_down_callback_add([&](const uint8_t x, const uint8_t y, const uint8 velocity) {
		cout << "버튼 눌림: " + to_string(x) + ", " + to_string(y) + ", " + to_string(velocity) << endl;
		// lp->velocity_set(x, y, velocity);
		lp->rgb_set(x, y, dis(gen), dis(gen), dis(gen));
		});
	const auto ibuc = lp->input_button_up_callback_add([&](const uint8_t x, const uint8_t y) {
		cout << "버튼 떼어짐: " + to_string(x) + ", " + to_string(y) << endl;
		// lp->velocity_set(x, y, 0);
		lp->rgb_set(x, y, 0, 0, 0);
	});

	cout << "종료하려면 엔터를 누르세요." << endl;
	cin.get();
	lp->input_button_down_callback_remove(ibdc);
	lp->input_button_up_callback_remove(ibuc);
	lp->program_mode_set(false);
	return 0;
}