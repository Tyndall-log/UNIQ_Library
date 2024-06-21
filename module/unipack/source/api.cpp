// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "unipack.h"

using namespace std;
using namespace uniq;
using namespace juce;

static shared_ptr<audio_device_manager> main_audio_device_manager;
static shared_ptr<launchpad> main_launchpad;
static shared_ptr<::uniq::uniq> main_uniq;
API void test()
{
	main_launchpad.reset();
	auto list = launchpad::get_available_output_list();
	auto input_list_legacy = MidiInput::getAvailableDevices();
	if (0 < input_list_legacy.size())
	{
		log::println("midi 입력 목록: ");
		for(auto i = 0; i < input_list_legacy.size(); ++i)
		{
			auto l = input_list_legacy[i];
			log::println("[" + to_string(i) + "]: " + l.name.toStdString());
			log::println("    -> id: " + l.identifier.toStdString());
		}
	}

	auto input_list = launchpad::get_available_input_list();
	if (!input_list.empty())
	{
		log::println("midi 입력 목록: ");
		for(auto i = 0; i < input_list.size(); ++i)
		{
			const auto& l = input_list[i];
			log::println("[" + to_string(i) + "]: " + l.name.toStdString());
			log::println("    -> id: " + l.identifier.toStdString());
		}
	}
	auto output_list = launchpad::get_available_output_list();
	if (!output_list.empty())
	{
		log::println("midi 출력 목록: ");
		for(auto i = 0; i < output_list.size(); ++i)
		{
			const auto& l = output_list[i];
			log::println("[" + to_string(i) + "]: " + l.name.toStdString());
			log::println("    -> id: " + l.identifier.toStdString());
		}
	}

	if (input_list.empty() && output_list.empty())
	{
		log::println("인식된 런치패드 없음.");
		return;
	}
	if (!main_audio_device_manager)
		main_audio_device_manager = audio_device_manager::create();
	main_launchpad = launchpad::create(main_audio_device_manager, &input_list[0], &output_list[0]);
	main_launchpad->program_mode_set(true);

	auto callback_id = main_launchpad->input_callback_add([](const uint8_t* data, int size)
	{
		log::println("MIDI> " + String::toHexString(data, size).toStdString());
	});

	if (main_uniq) main_uniq->launchpad_connect(main_launchpad);
}

API ::uniq::id_t unipack_load(const char* zip_file_path)
{
	log::println("unipack_load("s + zip_file_path + ")");
	main_uniq = unipack::unipack::load(zip_file_path);
	// if (main_launchpad)
	// 	main_uniq->launchpad_connect(main_launchpad);
	// else
	// 	main_uniq->launchpad_auto_connect();
	return main_uniq->ID_get();
}


// #if defined(ANDROID)
// extern "C" jint JNIEXPORT JNI_OnLoad(JavaVM* vm, void*)
// {
// 	__android_log_print(ANDROID_LOG_INFO, "uniq", "JNI_OnLoad()");
// 	//10초 대기
// 	// auto start = std::chrono::system_clock::now();
// 	// while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() < 10)
// 	// {
// 	// 	std::this_thread::sleep_for(std::chrono::milliseconds(100));
// 	// }
// 	return JNI_VERSION_1_6;
// }
// #endif