// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "api_launchpad.h"

namespace uniq::launchpad
{
#pragma region launchpad_manager
	API id_t *launchpad_list_get(size_t *size)
	{
		core::workspace_preset::workspace_info.reset();
		const auto manager = launchpad_manager::instance_get_without_creating();
		if (!manager)
		{
			*size = 0;
			log::error("create launchpad_manager before calling this function");
			return nullptr;
		}

		const auto list = manager->launchpad_list_get();
		*size = list.size();
		const auto result = new id_t[*size];
		for (size_t i = 0; i < *size; i++)
		{
			result[i] = list[i]->ID_get();
		}
		return result;
	}

	API void launchpad_list_get_delete(const id_t *launchpad_list)
	{
		delete[] launchpad_list;
	}
#pragma endregion launchpad_manager

#pragma region launchpad
	void rgb_set(const id_t launchpad_id, const uint8_t x, const uint8_t y, const uint8_t r, const uint8_t g, const uint8_t b)
	{
		const API_raii<launchpad> _launchpad(launchpad_id);
		if (!_launchpad) return;
		_launchpad->rgb_set(x, y, r, g, b);
	}

	void velocity_set(const id_t launchpad_id, const uint8_t x, const uint8_t y, const uint8_t velocity)
	{
		const API_raii<launchpad> _launchpad(launchpad_id);
		if (!_launchpad) return;
		_launchpad->velocity_set(x, y, velocity);
	}

	void program_mode_set(const id_t launchpad_id, const bool flag)
	{
		const API_raii<launchpad> _launchpad(launchpad_id);
		if (!_launchpad) return;
		_launchpad->program_mode_set(flag);
	}

	void automatic_transmission_set(const id_t launchpad_id, const bool flag)
	{
		const API_raii<launchpad> _launchpad(launchpad_id);
		if (!_launchpad) return;
		_launchpad->automatic_transmission_set(flag);
	}

	void immediate_transmission_set(const id_t launchpad_id, const bool flag)
	{
		const API_raii<launchpad> _launchpad(launchpad_id);
		if (!_launchpad) return;
		_launchpad->immediate_transmission_set(flag);
	}
#pragma endregion launchpad
}
