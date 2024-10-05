// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "id.h"

namespace uniq::core
{
	#pragma region ID_manager
	// std::unordered_map<id_t, std::any> ID_manager::registry_;
	std::unordered_map<id_t, std::tuple<std::any, id_t>> ID_manager::registry_;
	id_t ID_manager::id_ = static_cast<id_t>(api::predefined_ID::last);
	spin_lock ID_manager::lock_;

	id_t ID_manager::generate_ID()
	{
		std::unique_lock lock(lock_);
		id_t id = id_++;
		// registry_.emplace(id, std::any());
		registry_.emplace(id, std::tuple<std::any, id_t>());
		return id;
	}

	void ID_manager::unregister_ID(const id_t id)
	{
		std::unique_lock lock(lock_);
		registry_.erase(id);
	}
	#pragma endregion ID_manager
}