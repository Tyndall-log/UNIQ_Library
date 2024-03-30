// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

namespace uniq::audio::fade
{
	template <typename T>
	struct i_fade
	{
		virtual ~i_fade() = default;

		//get_gain(0) == 0, get_gain(1) == 1임을 보장해야 함
		[[nodiscard]] virtual T get_gain(T t) const = 0;
	};

	template <typename T>
	struct fade_linear final : i_fade<T>
	{
		T get_gain(T t) const override
		{
			return t;
		}
	};

	template <typename T>
	struct fade_ease_in_quad final : i_fade<T>
	{
		T get_gain(T t) const override
		{
			return t * t;
		}
	};

	template <typename T>
	struct fade_ease_out_quad final : i_fade<T>
	{
		T get_gain(T t) const override
		{
			return -t * (t - static_cast<T>(2));
		}
	};
}