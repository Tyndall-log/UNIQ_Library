// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "core.h"

using namespace std;
using namespace juce;

namespace uniq
{
#pragma region hierarchy
	id_t hierarchy_legacy::relationship_id_ = 0;
	
	hierarchy_legacy::~hierarchy_legacy()
	{
//		for (auto& child: child_list_)
//		{
//			child->child_remove(this);
//		}
		for (const auto& parent: parent_list_)
		{
			parent->child_remove(this);
		}
	}
#pragma endregion hierarchy
	
	/*template<typename T>
	SpinLock ID<T>::lock;*/
	
	MainMessageThread::MainMessageThread() : Thread("UNIQ_MessageThread")
	{
		startThread();
		log::println(wait(1000) ? "MainMessageThread start" : "MainMessageThread fail");
	}
	
	MainMessageThread::~MainMessageThread()
	{
		auto mm = MessageManager::getInstanceWithoutCreating();
		if (!mm) return;
		mm->stopDispatchLoop();
		log::println("MainMessageThread stop");
		stopThread(1000);
	}
	
	void MainMessageThread::run()
	{
		auto mm = unique_ptr<MessageManager>(MessageManager::getInstance());
		notify();
		mm->runDispatchLoop();
	}

	message_thread::message_thread(const bool current_thread_to_message_thread)
		: Thread("UNIQ_MessageThread")
	{
		current_thread_to_message_thread_ = current_thread_to_message_thread;
		if (current_thread_to_message_thread)
		{
			if (MessageManager::getInstanceWithoutCreating())
			{
				log::warn("message_thread already exists. delete it");
				MessageManager::deleteInstance();
			}
			mm_= unique_ptr<MessageManager>(MessageManager::getInstance());
			log::info("message_thread start");
			return;
		}
		startThread();
		log::info(wait(1000) ? "message_thread start" : "message_thread fail");
	}

	message_thread::~message_thread()
	{
		if (!mm_) return;
		mm_->stopDispatchLoop();
		if (current_thread_to_message_thread_)
		{
			[[maybe_unused]] const auto p = mm_.release();
			return;
		}
		const auto result = stopThread(1000);
		log::info(result ? "message_thread stop" : "message_thread stop fail");
	}

	void message_thread::run()
	{
#if defined(ANDROID)
		const auto env = getEnv();
		if (!env)
		{
			log::error("getEnv() failed");
			return;
		}
		jclass looperClass = env->FindClass("android/os/Looper");
		jmethodID prepareMethod = env->GetStaticMethodID(looperClass, "prepare", "()V");
		jmethodID loopMethod = env->GetStaticMethodID(looperClass, "loop", "()V");
		env->CallStaticVoidMethod(looperClass, prepareMethod);
		jclass handlerClass = env->FindClass("android/os/Handler");
		jmethodID handlerConstructor = env->GetMethodID(handlerClass, "<init>", "(Landroid/os/Looper;)V");
		jobject handler = env->NewObject(handlerClass, handlerConstructor, env->CallStaticObjectMethod(looperClass, env->GetStaticMethodID(looperClass, "myLooper", "()Landroid/os/Looper;")));
#endif
		mm_ = unique_ptr<MessageManager>(MessageManager::getInstance());
		notify(); // 메시지 스레드가 시작되었음을 알림
#if defined(ANDROID)
		env->CallStaticVoidMethod(looperClass, loopMethod);
		env->DeleteLocalRef(handler);
#else
		mm_->runDispatchLoop();
#endif
		mm_.reset();
		notify(); // 메시지 스레드가 종료되었음을 알림
		DeletedAtShutdown::deleteAll();
	}

	shared_ptr<message_thread> message_thread::get(const bool current_thread_to_message_thread)
	{
		lock_guard lock(mutex_);
		if (instance_) return instance_;
		if (!instance_weak_.expired()) return instance_weak_.lock();
		struct make_shared_enabler : message_thread
		{
			explicit make_shared_enabler(const bool current_thread_to_message_thread)
				: message_thread(current_thread_to_message_thread) {}
		};
		shared_ptr<message_thread> instance = make_shared<make_shared_enabler>(current_thread_to_message_thread);
		instance_weak_ = instance;
		return instance;
	}

	void message_thread::activate(const bool current_thread_to_message_thread)
	{
		lock_guard lock(mutex_);
		if (instance_) return;
		struct make_shared_enabler : message_thread
		{
			explicit make_shared_enabler(const bool current_thread_to_message_thread)
				: message_thread(current_thread_to_message_thread) {}
		};
		instance_weak_ = instance_ = make_shared<make_shared_enabler>(current_thread_to_message_thread);
	}

	void message_thread::deactivate()
	{
		lock_guard lock(mutex_);
		if (!instance_) return;
		instance_.reset();
	}

	// -----------------------------------------------------------------------------------------------
	// mutex_test
	std::shared_ptr<mutex_test> mutex_test::get()
	{
		std::lock_guard lock(mutex_);
		struct make_shared_enabler : mutex_test {};
		static std::shared_ptr<mutex_test> instance = std::make_shared<make_shared_enabler>();
		return instance;
	}
}
