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
	// as->play(player);
	auto max_num = 10;
	vector<shared_ptr<audio_segment>> segment_list;
	for (int i = 0; i < max_num; i++)
	{
		auto f = as->cue_add(44100 * i);
		if (!f)
		{
			cout << i << "번째 cue_add 실패" << endl;
			continue;
		}
		auto s = as->segment_create(44100 * i - 1);
		if (!s)
		{
			cout << i << "번째 segment_create 실패" << endl;
			continue;
		}
		segment_list.push_back(s);
	}
	auto last_s = as->segment_create(44100 * max_num - 1);
	// cin >> a;

	for(const auto &s : segment_list)
	{
		cin >> a;
		if (a == 0) continue;
		s->play(player);
		cout << "play" << endl;
	}
	cin >> a;
	cout << "마지막 segment_list" << endl;
	last_s->play(player);
	cin >> a;
	cout << "sync_test1 끝" << endl;
	// uniq::internal::audio_format_manager::format_manager_ = nullptr;
	return 0;
}

int sync_test2()
{
	cout << "sync_test2 시작" << endl;
	auto player = audio_player::create();
	auto as = audio_source::audio_load(audio_file_path.file3);
	cin.get();
	// as->play(player);
	auto max_num = 100;
	constexpr double t = 1.1653;
	constexpr double offset = 0;
	constexpr double split = 44100 * t;
	vector<shared_ptr<audio_segment>> segment_list;
	for (int i = 1; i <= max_num; i++)
	{
		as->cue_add(offset + split * i);
		auto s = as->segment_create(offset + split * i - 1);
		if (!s)
		{
			cout << i << "번째 segment_create 실패" << endl;
			continue;
		}
		// s->sync_duration_set(-5000ms, 5000ms);
		s->sync_duration_set(-999999ms, 5000ms);
		// s->sync_duration_set(0ms, 0ms);
		s->time_hint_set(static_cast<int>(t * i * 1e6 + offset) * 1us);
		if (!segment_list.empty()) s->sync_target_add(segment_list.back());
		cout << "segment id: " << s->ID_get() << endl;
		segment_list.push_back(s);
	}
	auto last_s = as->segment_create(offset + split * max_num + 1);

	for(const auto &s : segment_list)
	{
		cin.get();
		cout << s->ID_get() << " play" << endl;
		s->play(player);
	}
	cout << "마지막 segment_list" << endl;
	cin.get();
	cout << "나머지 재생" << endl;
	last_s->play(player);
	cin.get();
	cout << "sync_test2 끝" << endl;
	// uniq::internal::audio_format_manager::format_manager_ = nullptr;
	return 0;
}

int sync_test3()
{
	cout << "sync_test3 시작" << endl;
	auto player = audio_player::create();
	auto as = audio_source::audio_load(audio_file_path.file4);
	cin.get();
	// as->play(player);
	auto max_num = 600;
	// constexpr double t = 0.333;
	constexpr double t = 1;
	constexpr double offset = 44100 * 1.05;
	constexpr double split = 44100 * t;
	vector<shared_ptr<audio_segment>> segment_list;
	for (int i = 1; i <= max_num; i++)
	{
		as->cue_add(offset + split * i);
		auto s = as->segment_create(offset + split * i - 1);
		if (!s)
		{
			cout << i << "번째 segment_create 실패" << endl;
			continue;
		}
		s->sync_duration_set(-500ms, 500ms);
		// s->sync_duration_set(-999999ms, 1000ms);
		// s->sync_duration_set(0ms, 0ms);
		s->time_hint_set(static_cast<int>(t * i * 1e6) * 1us);
		if (!segment_list.empty())
		{
			s->sync_target_add(segment_list.back());
			// segment_list.back()->
		}
		cout << "segment id: " << s->ID_get() << endl;
		segment_list.push_back(s);
	}
	auto last_s = as->segment_create(offset + split * max_num + 1);

	for(const auto &s : segment_list)
	{
		// cin.get();
		cout << s->ID_get() << " play" << endl;
		cout << s->play(player) << endl;
	}
	cout << "마지막 segment_list" << endl;
	cin.get();
	cout << "나머지 재생" << endl;
	last_s->play(player);
	cin.get();
	cout << "sync_test3 끝" << endl;
	// uniq::internal::audio_format_manager::format_manager_ = nullptr;
	return 0;
}

int sync_test()
{
	cout << "sync_test 시작" << endl;
	// sync_test1();
	// sync_test2();
	sync_test3();
	return 0;
}
