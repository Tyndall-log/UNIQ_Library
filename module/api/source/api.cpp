// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "api.h"

using namespace std;
using namespace juce;
using namespace uniq;

#ifdef ANDROID
// extern "C" JNIEXPORT jint JNICALL
// JNI_OnLoad(JavaVM* vm, void* reserved)
// {
// 	log::info("JNI_OnLoad");
// 	return JNI_VERSION_1_6;
// }

// extern "C" JNIEXPORT void JNICALL
// Java_com_rmsl_juce_Java_juceInit(JNIEnv* env, jobject obj)
// {
// 	log::info("juceInit");
// 	Thread::initialiseJUCE(env, obj);
// }

// static shared_ptr<message_thread> mt;
// extern "C" JNIEXPORT void JNICALL
// Java_com_rmsl_juce_Java_messagesThreadRun()
// {
// 	mt = message_thread::get();
// }

extern "C" JNIEXPORT void JNICALL
Java_com_uniq_ui_Uniq_apiInit(JNIEnv* env, jobject jclass, jobject context)
{
	log::info("apiInit");
	__android_log_print(ANDROID_LOG_INFO, "uniq", "env: %p, jclass: %p, context: %p", env, jclass, context);
	JNIClassBase::initialiseAllClasses (env, context);
	Thread::initialiseJUCE(env, context);
	static shared_ptr<message_thread> mt = message_thread::get();
	log::info("apiInit end");
}

// API void api_init(const jobject context)
// {
// 	log::info("api_init");
// 	JNIEnv* env;
// 	vm->AttachCurrentThread(&env, nullptr);
// 	Thread::initialiseJUCE(env, context);
// 	vm->DetachCurrentThread();
// }
#endif
