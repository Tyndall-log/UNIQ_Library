// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "api.h"

namespace uniq::launchpad
{
	using namespace api;
#pragma region launchpad_manager
	API id_t *launchpad_list_get(size_t *size);
#pragma endregion launchpad_manager
#pragma region launchpad
	API void rgb_set(id_t launchpad_id, uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
	API void velocity_set(id_t launchpad_id, uint8_t x, uint8_t y, uint8_t velocity);
	API void program_mode_set(id_t launchpad_id, bool flag);
	API void automatic_transmission_set(id_t launchpad_id, bool flag);
	API void immediate_transmission_set(id_t launchpad_id, bool flag);
#pragma endregion launchpad
}
