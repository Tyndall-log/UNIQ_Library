// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "api.h"

using namespace std;
using namespace juce;
using namespace uniq;

#ifdef ANDROID

extern "C" JNIEXPORT void JNICALL
Java_com_uniq_ui_Uniq_apiInit(JNIEnv* env, jobject jclass, jobject context)
{
	log::info("apiInit");
	JNIClassBase::initialiseAllClasses (env, context);
	Thread::initialiseJUCE(env, context);
	static shared_ptr<message_thread> mt = message_thread::get();
}

#endif

void api::id_list_delete(const id_t *id_list)
{
	delete[] id_list;
}
