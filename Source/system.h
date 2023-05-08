// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "JuceHeader.h"
#include <algorithm>
#include <iostream>
#include <ranges>

using namespace std;
using namespace juce;

//콘솔에서 메인 스레드와 독립적으로 메시지 이벤트 처리할 수 있도록 하는 클래스
class MainMessageThread : public juce::Thread, public juce::Component
{
public:
	MainMessageThread();
	~MainMessageThread();

	void run() override;
};

//사용자 정의 리터럴을 구현합니다.

inline consteval unsigned char operator "" _uc(unsigned long long arg) noexcept { return static_cast<unsigned char>(arg); }

template<size_t N>
struct hex_convert
{
	uint8 data[N] = {};

	//template <size_t N>
	inline consteval hex_convert(const char(&s)[N], const char delim = '\'') noexcept
	{
		auto hex_char_to_uint8 = [](const char c) {
			if (c >= '0' && c <= '9')
				return static_cast<uint8>(c - '0');
			else if (c >= 'A' && c <= 'F')
				return static_cast<uint8>(c - 'A' + 10);
			else if (c >= 'a' && c <= 'f')
				return static_cast<uint8>(c - 'a' + 10);
			else
				return static_cast<uint8>(0);  // Unexpected character
		};

		uint8 tmp = 0;
		bool reading_first_digit = true;
		size_t data_idx = 0;
		for (size_t i = 0; i < N - 1; ++i)
		{
			if (s[i] == delim) continue;

			uint8 val = hex_char_to_uint8(s[i]);
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