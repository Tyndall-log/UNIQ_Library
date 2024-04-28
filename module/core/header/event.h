// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include <functional>
#include <map>

namespace uniq
{
	enum class callback_mode : std::uint8_t
	{
		change_before,
		change_after,
		remove_before,
	};

	/// @brief callback 함수를 저장하고 호출하는 클래스
	/// @tparam T
	/// @details T는 callback 함수의 인자 타입입니다.
	template<typename T>
	class callback_event
	{
		using callback_type = std::function<void(const T&)>;
		std::map<callback_mode, std::vector<callback_type>> callback_list_;
	public:
		callback_event() = default;
		~callback_event() = default;
		// callback_event(const callback_event&) = delete;
		// callback_event& operator=(const callback_event&) = delete;
		// callback_event(callback_event&&) = delete;
		// callback_event& operator=(callback_event&&) = delete;
		void add_callback(callback_type callback, callback_mode mode)
		{
			callback_list_[mode].emplace_back(callback);
		}
		void call_callback(const T& t, callback_mode mode)
		{
			for (const auto &callback : callback_list_[mode])
			{
				callback(t);
			}
		}
		bool remove_callback(callback_type callback, callback_mode mode)
		{
			auto list_it = callback_list_.find(mode);
			if (list_it == callback_list_.end())
				return false;
			auto it = std::find(list_it->second.begin(), list_it->second.end(), callback);
			if (it == list_it->second.end())
				return false;
			return true;
		}
	};

	enum class callback_check_mode : std::uint8_t
	{
		change_possible,
	};

	template<typename T>
	class callback_check_event
	{
		using callback_type = std::function<bool(const T&)>;
		std::map<callback_check_mode, std::vector<callback_type>> callback_list_;
	public:
		callback_check_event() = default;
		~callback_check_event() = default;
		// callback_check_event(const callback_check_event&) = delete;
		// callback_check_event& operator=(const callback_check_event&) = delete;
		// callback_check_event(callback_check_event&&) = delete;
		// callback_check_event& operator=(callback_check_event&&) = delete;
		void add_check_callback(callback_type callback, callback_check_mode mode)
		{
			callback_list_[mode].emplace_back(callback);
		}
		bool call_check_callback(const T& t, callback_check_mode mode)
		{
			for (const auto &callback : callback_list_[mode])
			{
				if (!callback(t))
					return false;
			}
			return true;
		}
		bool remove_check_callback(callback_type callback, callback_check_mode mode)
		{
			auto list_it = callback_list_.find(mode);
			if (list_it == callback_list_.end())
				return false;
			auto it = std::find(list_it->second.begin(), list_it->second.end(), callback);
			if (it == list_it->second.end())
				return false;
			return true;
		}
	};
}