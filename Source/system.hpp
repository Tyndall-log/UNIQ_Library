// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

namespace uniq
{
	//사용자 정의 리터럴을 구현합니다.
	inline consteval unsigned char operator "" _uc(unsigned long long arg) noexcept { return static_cast<unsigned char>(arg); }

	template<size_t N>
	struct hex_convert
	{
		juce::uint8 data[N] = {};

		inline consteval hex_convert(const char(&s)[N], const char delim = '\'') noexcept
		{
			auto hex_char_to_uint8 = [](const char c) {
				if (c >= '0' && c <= '9')
					return static_cast<juce::uint8>(c - '0');
				else if (c >= 'A' && c <= 'F')
					return static_cast<juce::uint8>(c - 'A' + 10);
				else if (c >= 'a' && c <= 'f')
					return static_cast<juce::uint8>(c - 'a' + 10);
				else
					return static_cast<juce::uint8>(0);  // Unexpected character
			};

			juce::uint8 tmp = 0;
			bool reading_first_digit = true;
			size_t data_idx = 0;
			for (size_t i = 0; i < N - 1; ++i)
			{
				if (s[i] == delim) continue;

				juce::uint8 val = hex_char_to_uint8(s[i]);
				if (reading_first_digit)
				{
					tmp = val << 4;
				}
				else
				{
					tmp |= val;
					data[data_idx++] = tmp;
				}

				reading_first_digit = !reading_first_digit;
			}
		}
	};

	template<hex_convert HC>
	inline consteval auto operator "" _hex() noexcept
	{
		return HC.data;
	}

	template<template<class...> class, typename>
	struct is_instance_of : public std::false_type {};

	template<template<class...> class T, typename... Us>
	struct is_instance_of<T, T<Us...>> : public std::true_type {};

	template<template<class...> class T, typename... Us>
	struct all_instances_of : public std::conjunction<is_instance_of<T, std::decay_t<Us>>...> {};

	template<template<class...> class T, typename... Us>
	struct any_instances_of : public std::disjunction<is_instance_of<T, std::decay_t<Us>>...> {};

	template<template<class...> class T, typename... Us>
	constexpr bool is_instance_of_v = is_instance_of<T, Us...>::value;

	template<template<class...> class T, typename... Us>
	constexpr bool all_instances_of_v = all_instances_of<T, Us...>::value;

	template<template<class...> class T, typename... Us>
	constexpr bool any_instances_of_v = any_instances_of<T, Us...>::value;
}