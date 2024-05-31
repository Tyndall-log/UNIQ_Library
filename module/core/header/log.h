// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include <string>
#include <iostream>
#include <mutex>
#include "lock.h"

namespace uniq
{
	class log
	{
		inline static auto sl = spin_lock();
		static std::string message_temp;
		static std::string message;
		static void println_without_lock(const std::string &str);
	public:
		struct time
		{
			std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
			std::string name;
			std::vector<std::pair<std::string, std::chrono::microseconds>> time_list;
			time(std::string_view name);
			~time();
			void stamp(const std::string& name);
		};
		static void print(std::string_view str);
		static void println(std::string_view str);
		static void info(std::string_view str);
		static void warn(std::string_view str);
		static void error(std::string_view str);
		[[nodiscard]] static std::unique_ptr<time> time(std::string_view name);
		static std::string& get();
	};
}
