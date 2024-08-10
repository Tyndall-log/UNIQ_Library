// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "hash.h"

namespace uniq::core::hash
{
	// constexpr unsigned long long fnv1a_hash(const char* str)
	// {
	// 	auto hash = 14695981039346656037ull;
	// 	while (*str) {
	// 		hash ^= static_cast<unsigned long long>(*str);
	// 		hash *= 1099511628211ull;
	// 		++str;
	// 	}
	// 	return hash;
	// }
}