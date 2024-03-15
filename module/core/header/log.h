// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include <string>
#include <iostream>

namespace uniq
{
	class log
	{
	public:
		static std::string message_temp;
		static std::string message;
		static void print(std::string_view str);
		static void println(std::string_view str);
		static std::string& get();
	};
}