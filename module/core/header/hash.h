// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include <string>

namespace uniq::core::hash
{
	// FNV-1a 해시 알고리즘을 사용하여 문자열을 해싱합니다.
	consteval unsigned long long fnv1a_hash(const std::string_view str)
	{
		auto hash = 14695981039346656037ull;
		for (auto &c : str)
		{
			hash ^= static_cast<unsigned long long>(c);
			hash *= 1099511628211ull;
		}
		return hash;
	}
}
