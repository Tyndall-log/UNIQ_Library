// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception


#include "log.h"
#ifdef ANDROID
#include <android/log.h>
#elif __APPLE__
#include <TargetConditionals.h>
#if defined(TARGET_OS_IOS)
#include <syslog.h>
#endif
#endif

using namespace std;

namespace uniq
{
	string log::message_temp;
	string log::message;

	void log::println_without_lock(const string &str)
	{
		string s;
		s += str;
		message += s + "\n";
#ifdef ANDROID
		__android_log_print(ANDROID_LOG_INFO, "uniq", "%s", s.data());
#else
		cout << s + "\n";
		cout.flush();
#endif
	}

	log::time::time(const string_view name) : name(name)
	{
	}

	log::time::~time()
	{
		lock_guard lock(sl);
		string s = "[Time]: ";
		s += name;
		s += " ";
		s += to_string(chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now() - start_time).count());
		s += "us";
		if (!time_list.empty())
		{
			for (auto &i : time_list)
			{
				s += "\n";
				s += i.first;
				s += " ";
				s += to_string(i.second.count());
				s += "us";
			}
		}
		println_without_lock(s);
	}

	void log::time::stamp(const std::string& name)
	{
		time_list.emplace_back(name, chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now() - start_time));
	}

	void log::print(string_view str)
	{
		lock_guard lock(sl);
		message += str;
#ifdef ANDROID
		__android_log_print(ANDROID_LOG_INFO, "uniq", "%s", str.data());
#else
		cout << str;
#endif
	}

	void log::println(const string_view str)
	{
		lock_guard lock(sl);
		string s;
		s += str;
		message += s + "\n";
#ifdef ANDROID
		__android_log_print(ANDROID_LOG_INFO, "uniq", "%s", s.data());
#else
		cout << s + "\n";
#endif
	}

	void log::info(const string_view str)
	{
		lock_guard lock(sl);
		string s = "[Info]: ";
		s += str;
		println_without_lock(s);
	}

	void log::warn(const string_view str)
	{
		lock_guard lock(sl);
		string s = "[Warn]: ";
		s += str;
		println_without_lock(s);
	}

	void log::error(const std::string_view str, const slm_t &slm)
	{
		lock_guard lock(sl);
		string s = "[Error]: ";
		s += slm.data_;
		s += " ";
		s += str;
		println_without_lock(s);
	}

	std::unique_ptr<struct log::time> log::time(std::string_view name)
	{
		return make_unique<struct time>(name);
	}

	string& log::get()
	{
		message_temp = message;
		message.clear();
		return message_temp;
	}
}
