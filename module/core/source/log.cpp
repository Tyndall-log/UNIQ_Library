// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception


#include "log.h"

using namespace std;

namespace uniq
{
	string log::message_temp;
	string log::message;

	void log::print(std::string_view str)
	{
		lock_guard lock(sl);
		message += str;
#ifdef ANDROID
		__android_log_print(ANDROID_LOG_INFO, "uniq", "%s", str.data());
#else
		cout << str;
#endif
	}

	void log::println(const std::string_view str)
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

	void log::info(const std::string_view str)
	{
		lock_guard lock(sl);
		string s = "[Info]: ";
		s += str;
		println(str);
	}

	void log::warn(const std::string_view str)
	{
		lock_guard lock(sl);
		string s = "[Warn]: ";
		s += str;
		println(str);
	}

	void log::error(const std::string_view str)
	{
		lock_guard lock(sl);
		string s = "[Error]: ";
		s += str;
		println(str);
	}

	std::string & log::get()
	{
		message_temp = message;
		message.clear();
		return message_temp;
	}
}
