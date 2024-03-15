// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception


#include "log.h"

namespace uniq
{
	std::string log::message_temp;
	std::string log::message;

	void log::print(std::string_view str)
	{
		message += str;
#ifdef ANDROID
			__android_log_print(ANDROID_LOG_INFO, "uniq", "%s", str.data());
#else
		std::cout << str;
#endif
	}

	void log::println(std::string_view str)
	{
		print(str);
		message += "\n";
#ifndef ANDROID
		std::cout << std::endl;
#endif

	}

	std::string & log::get()
	{
		message_temp = message;
		message.clear();
		return message_temp;
	}
}
