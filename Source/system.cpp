// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "system.h"

using namespace std;
using namespace juce;

namespace uniq
{
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



	/*template<typename T>
	juce::SpinLock ID<T>::lock;*/

	MainMessageThread::MainMessageThread() : Thread("UNIQ_MessageThread")
	{
		startThread();
		cout << (wait(1000) ? "MainMessageThread start" : "MainMessageThread fail") << endl;
	}

	MainMessageThread::~MainMessageThread()
	{
		MessageManager::getInstanceWithoutCreating()->stopDispatchLoop();
		stopThread(1000);
	}

	void MainMessageThread::run()
	{
		ScopedJuceInitialiser_GUI SJI_GUI;
		notify();
		MessageManager::getInstance()->runDispatchLoop();
		cout << "MainMessageThread stop" << endl;
	}
}