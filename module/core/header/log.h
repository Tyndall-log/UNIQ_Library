// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include <string>
#include <iostream>
#include <source_location>
#include "lock.h"

namespace uniq
{
	class log
	{
		struct source_location
		{
			using num_type = decltype(std::source_location::current().line());

			static consteval size_t push_data(char*& data, const std::string_view str)
			{
				if (data == nullptr) return str.size();
				for (const auto &c : str)
				{
					*data++ = c;
				}
				return str.size();
			}

			static consteval size_t push_data(char*& data, const num_type num)
			{
				int digit = 0;
				auto n = num;
				while (n)
				{
					n /= 10;
					++digit;
				}
				if (data == nullptr) return digit;
				n = num;
				for (int i = digit - 1; i >= 0; --i)
				{
					data[i] = static_cast<char>('0' + n % 10);
					n /= 10;
				}
				data += digit;
				return digit;
			}

			static consteval size_t make_data(char* data, const std::source_location location)
			{
				const std::string_view fulfile_name = location.file_name();
				constexpr std::string_view marker = "module/";
				size_t n = 0;
				n += push_data(data, fulfile_name.substr(fulfile_name.find(marker) + marker.length()));
				n += push_data(data, "(");
				n += push_data(data, location.line());
				n += push_data(data, ":");
				n += push_data(data, location.column());
				n += push_data(data, ") `");
				n += push_data(data, location.function_name());
				n += push_data(data, "`\0");
				return n;
			}

			// static consteval size_t get_N(const std::source_location location = std::source_location::current())
			// {
			// 	return make_data(nullptr, location);
			// }
		};

		template<size_t N>
		struct source_location_data
		{
			char data[N] = {};

			consteval explicit source_location_data(const std::source_location location = std::source_location::current())
			{
				source_location::make_data(data, location);
			}
		};

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
			explicit time(std::string_view name);
			~time();
			void stamp(const std::string& name);
		};
		static void print(std::string_view str);
		static void println(std::string_view str);
		static void info(std::string_view str);
		static void warn(std::string_view str);
		using source_info_t = source_location_data<4096>;
		static void error(std::string_view str, const source_info_t &source_info = source_info_t{});
		[[nodiscard]] static std::unique_ptr<time> time(std::string_view name);
		static std::string& get();
	};
}
