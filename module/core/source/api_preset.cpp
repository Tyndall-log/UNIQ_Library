// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "api_preset.h"

using namespace std;
using namespace uniq;

#ifdef UNIQ_DLL_API
namespace uniq::core::api
{
	mutex global_api_lock;
	API_callback_manager callback_manager;

	// workspace_info_raii::workspace_info_raii(const id_t id, const source_location location)
	// {
	// 	if (id < static_cast<id_t>(predefined_ID::last))
	// 	{
	// 		log::error("workspace_id is less than predefined_ID::last");
	// 		failed = true;
	// 		return;
	// 	}
	// 	workspace_preset::workspace_info.set(id, location);
	// }
	//
	// bool workspace_info_raii::is_failed() const
	// {
	// 	return failed;
	// }
	//
	// workspace_info_raii::~workspace_info_raii()
	// {
	// 	workspace_preset::workspace_info.reset();
	// }

	void API_callback_manager::add_create_ID(const id_t create_id, const ID_info<> &info)
	{
		const auto msg = new API_callback_message(
			static_cast<id_t>(predefined_ID::create),
			info.type_hash,
			new ID_lifecycle{create_id});
		msg->set_func_name(info.type_name);
		add(msg);
	}

	void API_callback_manager::add_destroy_ID(const id_t destroy_id, const ID_info<> &info)
	{
		const auto msg = new API_callback_message(
			static_cast<id_t>(predefined_ID::destroy),
			info.type_hash,
			new ID_lifecycle{destroy_id});
		msg->set_func_name(info.type_name);
		add(msg);
	}

	const API_callback_message_base* API_callback_manager::pop_get()
	{
		unique_lock lock(lock_);
		if (callback_message_queue.empty())
			return nullptr;
		const auto msg = callback_message_queue.front();
		callback_message_queue.pop();
		return msg;
	}

	API const void* API_callback_message_get()
	{
		const auto &m = callback_manager.pop_get();
		if (m == nullptr) return nullptr;
		return &m->api_workspace_id;
	}

	static const auto API_callback_message_struct_offset =
		reinterpret_cast<uintptr_t>(&reinterpret_cast<API_callback_message_base*>(0)->api_workspace_id);
	API void API_callback_message_release(const void* const msg)
	{
		const auto _msg = reinterpret_cast<uintptr_t>(msg) - API_callback_message_struct_offset;
		delete reinterpret_cast<API_callback_message_base*>(_msg);
	}
}

API int test1(const int a, const int b)
{
	return a + b;
}

API const char* log_get()
{
	return log::get().c_str();
}

API void log_clear()
{
	log::get().clear();
}



#endif