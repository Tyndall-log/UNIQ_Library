// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "core.h"

using namespace std;
using namespace juce;

namespace uniq
{
#ifdef _WIN32
#define API extern "C" __declspec(dllexport)
#else
#define API extern "C" __attribute__((visibility("default")))
#endif
	std::string log::message_temp;
	std::string log::message;
	
#pragma region ID_manager
	std::size_t ID_manager::id_ = 0;
	std::unordered_map<std::size_t, std::any> ID_manager::registry_;
	juce::SpinLock ID_manager::lock_;
	
	std::size_t ID_manager::generate_ID()
	{
		juce::SpinLock::ScopedLockType scoped_lock(lock_);
		std::size_t id = ++id_;
		registry_.emplace(id, std::any());
		return id;
	}
	
	void ID_manager::unregister_ID(std::size_t id)
	{
		juce::SpinLock::ScopedLockType scoped_lock(lock_);
		registry_.erase(id);
	}
#pragma endregion ID_manager

#pragma region hierarchy
	std::size_t hierarchy::relationship_id_ = 0;
	
	hierarchy::~hierarchy()
	{
//		for (auto& child: child_list_)
//		{
//			child->child_remove(this);
//		}
		for (auto& parent: parent_list_)
		{
			parent->child_remove(this);
		}
	}
#pragma endregion hierarchy
	
	/*template<typename T>
	juce::SpinLock ID<T>::lock;*/
	
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
}