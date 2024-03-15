// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "main.h"


namespace ns_test3
{
	using namespace std;
	using namespace uniq;

}

using namespace ns_test3;


#include <random>

void thread_deadlock_test()
{
	shared_recursive_timed_mutex_priority lock;
	// shared_mutex lock;
	int global_count = 0;
	atomic_int sum_local_count = 0;

	auto func = [&](int id) {
		int lock_count = 0;
		int lock_mode = 0; // 0: unlock, 1: lock, 2: lock_shared

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);
		std::uniform_int_distribution<> dis_int(0, 1);

		auto len = 1'000'000;
		auto step = 100'000;
		auto count = 0;
		for (int i = 0; i < len; ++i)
		{
			if (i % step == 0)
				std::cout << "Thread " + to_string(id) + " " + to_string(i / step) + "\n";

			double probability = 1.0 / (lock_count + 1);
			double random_value = dis(gen);

			if (random_value < probability) //lock
			{
				if (lock_mode == 0)
				{
					if (dis_int(gen) == 0)
						lock_mode = 1;
					else
						lock_mode = 2;
				}

				++lock_count;
				if (lock_mode == 1)
				{
					// cout << to_string(id)+" lock\n";
					lock.lock();
				}
				else
				{
					// cout << to_string(id)+" lock_shared\n";
					lock.lock_shared();
				}
			}
			else
			{
				if (lock_mode == 1)
				{
					// cout << to_string(id)+" unlock\n";
					lock.unlock();
				}
				else
				{
					// cout << to_string(id)+" unlock_shared\n";
					lock.unlock_shared();
				}
				if (--lock_count <= 0)
				{
					lock_mode = 0;
				}
			}

			if (lock_mode == 1)
			{
				++global_count;
				++count;
			}
		}

		if (lock_mode == 1)
			while (0 < lock_count)
			{
				--lock_count;
				lock.unlock();
			}
		else
			while (0 < lock_count)
			{
				--lock_count;
				lock.unlock_shared();
			}

		sum_local_count += count;

		std::cout << "Thread " << id << " completed." << std::endl;
	};

	std::vector<std::thread> threads;

	for (int i = 0; i < 8; ++i)
	{
		threads.emplace_back(func, i);
	}

	for (auto &t: threads)
	{
		t.join();
	}

	std::cout << "All threads completed." << std::endl;
	std::cout << "global_count: " << global_count << std::endl;
	std::cout << "sum_local_count: " << sum_local_count << std::endl;
}


int lock_test()
{
	log::println("lock_test");

	using qd = shared_recursive_timed_mutex_priority::qd;
	using qd_compare = shared_recursive_timed_mutex_priority::qd_compare;
	// std::priority_queue<qd, std::vector<qd>, std::greater<>> queue_;
	// std::priority_queue<qd*, std::vector<qd*>, qd_compare> queue_;
	std::priority_queue<qd *, std::vector<qd *>, qd_compare> queue_;

	//  qd qd1(0);
	// queue_.push(qd1);
	auto start = std::chrono::steady_clock::now();
	queue_.emplace(new qd);
	queue_.emplace(new qd);
	queue_.emplace(new qd(-1));
	queue_.emplace(new qd);

	auto i_max = queue_.size();
	for (int i = 0; i < i_max; ++i)
	{
		// 시간 표시
		auto time = queue_.top()->time_;
		auto duration = time - start;
		cout << "duration: " << duration.count() << endl;
		queue_.pop();
	}

	// atomic_flag 테스트
	std::atomic_flag flag = ATOMIC_FLAG_INIT;


	auto t = thread([&] {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		flag.test_and_set();
		cout << "thread end" << endl;
		flag.notify_one();
		start = std::chrono::steady_clock::now();
	});
	t.detach();

	flag.wait(false);
	auto time = std::chrono::steady_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	auto duration = time - start;
	cout << "duration: " << static_cast<float>(chrono::duration_cast<chrono::nanoseconds>(duration).count()) / 1000 <<
			"(us)" << endl;

	// spin_lock 테스트
	spin_lock sl;
	sl.lock();
	t = thread([&] {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		cout << "thread end" << endl;
		sl.unlock();
		start = std::chrono::steady_clock::now();
	});
	t.detach();
	sl.lock();
	time = std::chrono::steady_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	duration = time - start;
	cout << "duration: " << static_cast<float>(chrono::duration_cast<chrono::nanoseconds>(duration).count()) / 1000 <<
			"(us)" << endl;


	//pd 객체 순서 확인
	{
		using pd = shared_recursive_timed_mutex_priority::qd;
		pd pd1(0);
		pd pd2(1);
		pd pd3(-1);
		pd pd4(0);

		cout << "pd1 -> pd2: " << (pd1 < pd2) << endl;
		cout << "pd1 -> pd3: " << (pd1 < pd3) << endl;
		cout << "pd1 -> pd4: " << (pd1 < pd4) << endl;
		cout << "pd2 -> pd3: " << (pd2 < pd3) << endl;
		cout << "pd2 -> pd4: " << (pd2 < pd4) << endl;
		cout << "pd3 -> pd4: " << (pd3 < pd4) << endl;
		cout << "pd3 -> pd1 -> pd4 -> pd2: " << (pd3 < pd1 && pd1 < pd4 && pd4 < pd2) << endl;
	}

	//빠른 표준 출력
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);

	thread_deadlock_test();

	return 0;
}
