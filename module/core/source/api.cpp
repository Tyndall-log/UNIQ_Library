// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "core.h"

using namespace std;
using namespace uniq;
using namespace juce;

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

static std::shared_ptr<message_thread> mt;
extern "C" JNIEXPORT void JNICALL
Java_com_rmsl_juce_Java_messagesThreadRun()
{
	mt = message_thread::get();
}
#endif

API const char* log_get()
{
	return log::get().c_str();
}

API int test1(int a, int b)
{
	return a + b;
}
