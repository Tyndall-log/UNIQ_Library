// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "main.h"

using namespace std;
using namespace juce;

class UNIQ_Library_workspace_test final : public JUCEApplicationBase
{
	const String getApplicationName() override { return "UNIQ_Library_workspace_test"; }
	const String getApplicationVersion() override { return "0.0.0"; }
	bool moreThanOneInstanceAllowed() override { return false; }
	void anotherInstanceStarted(const String&) override {}
	void initialise(const String&) override {}
	void shutdown() override {}
	void systemRequestedQuit() override {}
	void unhandledException(const std::exception*, const String&, int) override {}
	void suspended() override {}
	void resumed() override {}
};

int main(const int argc, const char* argv[])
{
#if defined(_WIN32)
	system("chcp 65001"); //utf-8
#endif

	//force the main thread to be the current thread
	auto mm = uniq::message_thread::get(true);

	auto t = std::thread([&] {
		workspace_test1();
		mm.reset();
	});

	t.detach();
	JUCEApplicationBase::createInstance = []() -> JUCEApplicationBase* { return new UNIQ_Library_workspace_test(); };
	return JUCEApplicationBase::main(argc, argv);
}