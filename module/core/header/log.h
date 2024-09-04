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
		template<size_t N = 4096>
		class source_location_marker
		{
			using num_type = decltype(std::source_location::current().line());
			// using char_type = decltype(std::source_location::current().function_name());
		public:
			char data_[N] = {};
			num_type line_{};
			num_type column_{};
			size_t file_name_start_{};
			size_t file_name_end_{};
			size_t function_name_start_{};
			size_t function_name_end_{};

		private:
			static consteval size_t push_data(char*& data, const std::string_view str, std::string_view *str_ref = nullptr)
			{
				if (data == nullptr) return str.size();
				if (str_ref != nullptr)
				{
					*str_ref = std::string_view(data, str.size());
				}
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

			consteval size_t make_data(const std::source_location location)
			{
				const std::string_view fulfile_name = location.file_name();
				constexpr std::string_view marker = "module/";
				size_t n = 0;
				auto _data = &data_[n];
				file_name_start_ = n;
				n += push_data(_data, fulfile_name.substr(fulfile_name.find(marker) + marker.length()));
				file_name_end_ = n;
				n += push_data(_data, "(");
				n += push_data(_data, location.line());
				n += push_data(_data, ":");
				n += push_data(_data, location.column());
				n += push_data(_data, ") `");
				function_name_start_ = n;
				n += push_data(_data, location.function_name());
				function_name_end_ = n;
				n += push_data(_data, "`\0");
				return n;
			}
		public:
			consteval explicit source_location_marker(
				const std::source_location location = std::source_location::current())
				: line_(location.line()), column_(location.column())
			{
				make_data(location);
			}
			[[nodiscard]] constexpr num_type line() const { return line_; }
			[[nodiscard]] constexpr num_type column() const { return column_; }
			[[nodiscard]] constexpr std::string_view file_name() const { return std::string_view(data_ + file_name_start_, file_name_end_ - file_name_start_); }
			[[nodiscard]] constexpr std::string_view function_name() const { return std::string_view(data_ + function_name_start_, function_name_end_ - function_name_start_); }
		};

		// template<size_t N = 4096>
		// struct source_location_data
		// {
		// 	char data[N] = {};
		//
		// 	consteval explicit source_location_data(source_location_marker<> location = source_location_marker<>{})
		// 	{
		// 		make_data(data, location);
		// 	}
		// };

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
		using slm_t = source_location_marker<>;
		static void print(std::string_view str);
		static void println(std::string_view str);
		static void info(std::string_view str);
		static void warn(std::string_view str);
		static void error(std::string_view str, const slm_t &slm = slm_t{});
		[[nodiscard]] static std::unique_ptr<time> time(std::string_view name);
		static std::string& get();
	};
}
