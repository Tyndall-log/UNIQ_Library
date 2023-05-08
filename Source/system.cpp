// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "system.h"

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