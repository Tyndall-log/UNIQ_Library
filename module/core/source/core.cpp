// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "core.h"

using namespace std;
using namespace juce;

namespace uniq
{

#pragma region ID_manager
	id_t ID_manager::id_ = static_cast<id_t>(core::api::predefined_ID::last);
	std::unordered_map<id_t, std::any> ID_manager::registry_;
	spin_lock ID_manager::lock_;

	id_t ID_manager::generate_ID()
	{
		std::unique_lock lock(lock_);
		id_t id = id_++;
		registry_.emplace(id, std::any());
		return id;
	}
	
	void ID_manager::unregister_ID(const id_t id)
	{
		std::unique_lock lock(lock_);
		registry_.erase(id);
	}
#pragma endregion ID_manager

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

	message_thread::message_thread()
#ifdef _WIN32
	: Thread("UNIQ_MessageThread")

	{
		startThread();
		log::info(wait(1000) ? "message_thread start" : "message_thread fail");
	}
#else
	{
		mm_ = unique_ptr<MessageManager>(MessageManager::getInstance());
		mm_->runDispatchLoop();
		log::info("message_thread start");
	}

	message_thread::~message_thread()
	{
		if (!mm_) return;
		mm_->stopDispatchLoop();
		mm_.reset();
		log::info("message_thread stop");
	}
#endif

#ifdef _WIN32
	message_thread::~message_thread()
	{
		if (!mm_) return;
		mm_->stopDispatchLoop();
		const auto result = stopThread(1000);
		log::info(result ? "message_thread stop" : "message_thread stop fail");
	}

	void message_thread::run()
	{
		mm_ = unique_ptr<MessageManager>(MessageManager::getInstance());
		notify(); // 메시지 스레드가 시작되었음을 알림
		mm_->runDispatchLoop();
		notify(); // 메시지 스레드가 종료되었음을 알림
		mm_.reset();
		DeletedAtShutdown::deleteAll();
	}
#endif

	shared_ptr<message_thread> message_thread::get()
	{
		lock_guard lock(mutex_);
		if (instance_) return instance_;
		if (!instance_weak_.expired()) return instance_weak_.lock();
		struct make_shared_enabler : message_thread {};
		shared_ptr<message_thread> instance = make_shared<make_shared_enabler>();
		instance_weak_ = instance;
		return instance;
	}

	void message_thread::activate()
	{
		lock_guard lock(mutex_);
		if (instance_) return;
		struct make_shared_enabler : message_thread {};
		instance_weak_ = instance_ = make_shared<make_shared_enabler>();
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
