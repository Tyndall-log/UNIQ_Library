// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include <unordered_set>
#include <source_location>

#include "alias.h"
#include "log.h"

namespace uniq::core::workspace_preset
{
	class workspace_info_class
	{
		inline static std::string_view default_name = "unknown_API_function_name";

		/// 현재 API 함수가 호출되는 workspace의 ID입니다.<br>
		/// workspace_id는 API 함수가 호출되는 동안에만 유효합니다.
		id_t id = 0;

		/// API 함수의 이름입니다.
		std::string_view api_function_name = default_name;

	public:

		void set(id_t id, const log::slm_t &location = log::slm_t{});

		void reset();

		[[nodiscard]] id_t get_id() const;

		[[nodiscard]] std::string_view get_function_name() const;
	};

	extern thread_local workspace_info_class workspace_info;
	extern std::unordered_set<id_t> workspace_id_set;

}
