// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "core.h"

using namespace std;
using namespace uniq;
using namespace juce;

#ifdef UNIQ_DLL_API
namespace uniq::core::api
{
	mutex global_api_lock;
	API_callback_manager callback_manager;

	workspace_info_raii::workspace_info_raii(const id_t id, const source_location location)
	{
		workspace_preset::workspace_info.set(id, location);
	}

	workspace_info_raii::~workspace_info_raii()
	{
		workspace_preset::workspace_info.reset();
	}

	// void API_callback_message_legacy::set_obj_id_type_name(const char *type_name)
	// {
	// 	obj_id_type_name = type_name;
	// }
	//
	// void API_callback_message_legacy::set_func_name(const char *func_name)
	// {
	// 	func_id_name = func_name;
	// }

	// void API_callback_manager::add(const API_callback_message_legacy* const msg)
	// {
	// 	unique_lock lock(lock_);
	// 	callback_queue.push(msg);
	// }
	//
	// void API_callback_manager::add_create_ID(const id_t create_id, const ID_info<> &info)
	// {
	// 	const auto msg = new API_callback_message_legacy(static_cast<id_t>(predefined_ID::create), info.type_hash, create_id);
	// 	msg->set_func_name(info.type_name);
	// 	// log::info("add_create_ID: "s+info.type_name + ", " + to_string(info.type_hash));
	// 	add(msg);
	// }
	//
	// void API_callback_manager::add_destroy_ID(const id_t destroy_id, const ID_info<> &info)
	// {
	// 	const auto msg = new API_callback_message_legacy(static_cast<id_t>(predefined_ID::destroy), info.type_hash, destroy_id);
	// 	msg->set_func_name(info.type_name);
	// 	// log::info("add_destroy_ID: "s+info.type_name + ", " + to_string(info.type_hash));
	// 	add(msg);
	// }

	void API_callback_manager::add_create_ID(const id_t create_id, const ID_info<> &info)
	{
		const auto msg = new API_callback_message(
			static_cast<id_t>(predefined_ID::create),
			info.type_hash,
			new ID_lifecycle{create_id});
		msg->set_func_name(info.type_name);
		// log::info("add_create_ID: "s+info.type_name + ", " + to_string(info.type_hash));
		add(msg);
	}

	void API_callback_manager::add_destroy_ID(const id_t destroy_id, const ID_info<> &info)
	{
		const auto msg = new API_callback_message(
			static_cast<id_t>(predefined_ID::destroy),
			info.type_hash,
			new ID_lifecycle{destroy_id});
		msg->set_func_name(info.type_name);
		// log::info("add_destroy_ID: "s+info.type_name + ", " + to_string(info.type_hash));
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
		return &callback_manager.pop_get()->api_workspace_id;
	}

	static const auto API_callback_message_struct_offset =
		reinterpret_cast<uintptr_t>(&reinterpret_cast<API_callback_message_base*>(0)->api_workspace_id);
	API void API_callback_message_release(const void* const msg)
	{
		const auto _msg = reinterpret_cast<uintptr_t>(msg) - API_callback_message_struct_offset;
		delete reinterpret_cast<API_callback_message_base*>(_msg);
	}
}

#ifdef ANDROID
// extern "C" JNIEXPORT jint JNICALL
// JNI_OnLoad(JavaVM* vm, void* reserved)
// {
// 	log::info("JNI_OnLoad");
// 	return JNI_VERSION_1_6;
// }


extern "C" JNIEXPORT void JNICALL
Java_com_rmsl_juce_Java_juceInit(JNIEnv* env, jobject obj)
{
	log::info("juceInit");
	Thread::initialiseJUCE(env, obj);
}

static shared_ptr<message_thread> mt;
extern "C" JNIEXPORT void JNICALL
Java_com_rmsl_juce_Java_messagesThreadRun()
{
	mt = message_thread::get();
}
#endif

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