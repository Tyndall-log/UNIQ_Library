// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

namespace uniq::audio::interpolator
{
	class catmull_rom
	{
	public:
		// p1 ~ p2 사이에서 t(0~1)에 대한 보간값을 반환
		template <typename T>
		static T interpolate(const T* p, const T t)
		{
			const auto& c_0_5 = static_cast<T>(0.5);
			const auto& c_2 = static_cast<T>(2);
			const auto& c_3 = static_cast<T>(3);
			const auto& c_4 = static_cast<T>(4);
			const auto& c_5 = static_cast<T>(5);
			const auto& p0 = p[0];
			const auto& p1 = p[1];
			const auto& p2 = p[2];
			const auto& p3 = p[3];
			return p1 + t * (p2 - p0
			          + t * (c_2 * p0 - c_5 * p1 + c_4 * p2 - p3
			          + t * (c_3 * (p1 - p2) + p3 - p0)))
			          * c_0_5;
		}
	};
}

