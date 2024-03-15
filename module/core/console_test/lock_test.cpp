// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "main.h"


namespace ns_test3
{
	using namespace std;
	using namespace uniq;

	class shared_recursive_timed_mutex
	{
		std::timed_mutex mutex_;
		spin_lock member_access_lock_;
		std::thread::id exclusive_owner_;
		std::size_t exclusive_count_ = 0;
		std::unordered_map<std::thread::id, std::size_t> shared_owners_;

		void lock()
		{
			const auto this_id = this_thread::get_id();
			unique_lock lock(member_access_lock_);
			if (exclusive_owner_ == this_id)
			{
				++exclusive_count_;
				return;
			}
			if (shared_owners_.contains(this_id)) // 이미 현재 스레드가 공유 잠금을 가지고 있는 경우
			{
				if (shared_owners_.size() == 1) // 현재 스레드만 공유 잠금을 가지고 있는 경우
				{
					// 공유 잠금을 독점 잠금으로 업그레이드
					exclusive_owner_ = this_id;
					exclusive_count_ = 1;
					return;
				}
			}
			lock.unlock();
			mutex_.lock(); // 독점 잠금을 시도
			lock.lock();
			exclusive_owner_ = this_id;
			exclusive_count_ = 1;
		}

		bool try_lock() noexcept
		{
			const auto this_id = this_thread::get_id();
			unique_lock lock(member_access_lock_);
			if (exclusive_owner_ == this_id)
			{
				++exclusive_count_;
				return true;
			}
			if (shared_owners_.contains(this_id)) // 이미 현재 스레드가 공유 잠금을 가지고 있는 경우
			{
				if (shared_owners_.size() == 1) // 현재 스레드만 공유 잠금을 가지고 있는 경우
				{
					// 공유 잠금을 독점 잠금으로 업그레이드
					exclusive_owner_ = this_id;
					exclusive_count_ = 1;
					return true;
				}
			}
			lock.unlock();
			if (mutex_.try_lock()) // 독점 잠금을 시도
			{
				lock.lock();
				exclusive_owner_ = this_id;
				exclusive_count_ = 1;
				return true;
			}
			return false;
		}

		void unlock()
		{
			const auto this_id = this_thread::get_id();
			unique_lock lock(member_access_lock_);
			if (exclusive_owner_ == this_id)
			{
				if (--exclusive_count_ <= 0)
				{
					exclusive_owner_ = std::thread::id();
					exclusive_count_ = 0;
					if (shared_owners_.empty()) // 현재 스레드가 공유 잠금을 가지고 있지 않은 경우
					{
						mutex_.unlock(); // 독점 잠금 해제
					}
				}
			}
		}

		// void lock_shared()
		// {
		// 	const auto this_id = this_thread::get_id();
		// 	unique_lock lock(member_access_lock_);
		// 	if (exclusive_owner_ == this_id)
		// 	{
		// 		auto [iter, success] = shared_owners_.try_emplace(this_id, 1);
		// 		if (!success)
		// 		{
		// 			++iter->second;
		// 		}
		// 		return;
		// 	}
		// 	auto iter = shared_owners_.find(this_id);
		// 	if (iter != shared_owners_.end()) // 이미 현재 스레드가 공유 잠금을 가지고 있는 경우
		// 	{
		// 		++iter->second;
		// 		return;
		// 	}
		// 	lock.unlock();
		// 	mutex_.lock_shared(); // 공유 잠금을 시도
		// 	lock.lock();
		// 	shared_owners_[this_id] = 1;
		// }
	};

	// class shared_recursive_timed_mutex_priority
	// {
	// public:
	// 	struct qd
	// 	{
	// 		int priority_ = 0;
	// 		std::thread::id id_ = std::thread::id();
	// 		std::chrono::time_point<std::chrono::steady_clock> time_ = std::chrono::steady_clock::now();
	// 		std::atomic_flag flag_ = {};
	//
	// 		//우주선 연산자
	// 		auto operator<=>(const qd& other) const
	// 		{
	// 			if (const auto result = priority_ <=> other.priority_; result != 0)
	// 				return result;
	// 			return time_ <=> other.time_;
	// 		}
	// 	};
	//
	// 	struct qd_compare
	// 	{
	// 		bool operator()(const qd* lhs, const qd* rhs) const
	// 		{
	// 			if (lhs->priority_ != rhs->priority_)
	// 				return lhs->priority_ > rhs->priority_;
	// 			return lhs->time_ > rhs->time_;
	// 		}
	// 	};
	//
	// 	std::priority_queue<qd*, std::vector<qd*>, qd_compare> exclusive_queue_;
	// 	std::priority_queue<qd*, std::vector<qd*>, qd_compare> shared_queue_;
	// 	spin_lock member_access_lock_;
	// 	std::thread::id exclusive_owner_;
	// 	std::size_t exclusive_count_ = 0;
	// 	std::unordered_map<std::thread::id, std::size_t> shared_owners_;
	// 	const std::chrono::milliseconds exclusive_maximum_wait_time_ = std::chrono::milliseconds(1000);
	//
	// private:
	//
	// 	/// exclusive_wake_up는 member_access_lock_를 잠금한 상태에서 호출해야 한다.
	// 	void exclusive_wake_up()
	// 	{
	// 		auto &_qd = exclusive_queue_.top();
	// 		exclusive_owner_ = _qd->id_;
	// 		exclusive_count_ = 1;
	// 		_qd->flag_.test_and_set();
	// 		_qd->flag_.notify_one();
	// 		exclusive_queue_.pop();
	// 	}
	//
	// 	/// shared_wake_up는 member_access_lock_를 잠금한 상태에서 호출해야 한다.
	// 	void shared_wake_up()
	// 	{
	// 		while (!shared_queue_.empty())
	// 		{
	// 			auto &_qd = shared_queue_.top();
	// 			const auto [iter, success] = shared_owners_.try_emplace(_qd->id_, 1);
	// 			if (!success) ++iter->second;
	// 			_qd->flag_.test_and_set();
	// 			_qd->flag_.notify_one();
	// 			shared_queue_.pop();
	// 		}
	// 	}
	//
	// public:
	// 	~shared_recursive_timed_mutex_priority()
	// 	{
	// 		while (!exclusive_queue_.empty())
	// 		{
	// 			delete exclusive_queue_.top();
	// 			exclusive_queue_.pop();
	// 		}
	// 		while (!shared_queue_.empty())
	// 		{
	// 			delete shared_queue_.top();
	// 			shared_queue_.pop();
	// 		}
	// 	}
	//
	// 	// 경우의 수 정리
	// 	// e는 exclusive, s는 shared
	// 	// x는 없음, 나는 현재 스레드, 다른은 다른 스레드, all은 현재 스레드와 다른 스레드
	// 	// e:x -> s:x -> 가능
	// 	//        s:나 -> 가능
	// 	//        s:다른 -> 가능
	// 	//        s:all -> 가능
	// 	// e:나 -> s:x -> 가능
	// 	//         s:나 -> 가능
	// 	//         s:다른 -> 불가능
	// 	//         s:all -> 불가능
	// 	// e:다른 -> s:x -> 가능
	// 	//           s:나 -> 불가능
	// 	//           s:다른 -> 가능
	// 	//           s:all -> 불가능
	//
	// 	void lock(const int priority = 0)
	// 	{
	// 		//e:x -> s:x -> 즉시
	// 		//       s:나 -> 즉시
	// 		//       s:다른 -> wait
	// 		//       s:all -> wait
	// 		//e:나 -> e++
	// 		//e:다른 -> wait
	// 		const auto this_id = this_thread::get_id();
	// 		unique_lock lock(member_access_lock_);
	// 		if (exclusive_owner_ == this_id)
	// 		{
	// 			++exclusive_count_;
	// 			return;
	// 		}
	// 		if (exclusive_owner_ == std::thread::id())
	// 		{
	// 			if (shared_owners_.empty() || (shared_owners_.size() == 1 && shared_owners_.contains(this_id)))
	// 			{
	// 				exclusive_owner_ = this_id;
	// 				exclusive_count_ = 1;
	// 				return;
	// 			}
	// 		}
	// 		auto _qd = new qd(priority, this_id);
	// 		exclusive_queue_.push(_qd);
	// 		lock.unlock();
	// 		_qd->flag_.wait(false);
	// 		delete _qd;
	// 	}
	//
	// 	void unlock()
	// 	{
	// 		//e:x -> 무시
	// 		//e:나 -> s:x -> e--
	// 		//        s:나 -> e--
	// 		//e:다른 -> 무시
	// 		const auto this_id = this_thread::get_id();
	// 		unique_lock lock(member_access_lock_);
	// 		if (exclusive_owner_ != this_id || 0 < --exclusive_count_)
	// 			return;
	//
	// 		exclusive_owner_ = std::thread::id();
	// 		exclusive_count_ = 0;
	//
	// 		//현재 상태의 경우의 수
	// 		//e:x -> s:x -> ec ? e_notify : (e_q < s_q ? e_notify : s_notify)
	// 		//       s:나 -> 무시
	//
	// 		if (shared_owners_.contains(this_id))
	// 		{
	// 			shared_wake_up();
	// 			return;
	// 		}
	//
	// 		//스래드 깨우기
	// 		if (!exclusive_queue_.empty())
	// 		{
	// 			auto now = std::chrono::steady_clock::now();
	// 			bool exclusive_condition = exclusive_maximum_wait_time_ <= now - exclusive_queue_.top()->time_;
	// 			if (exclusive_condition
	// 			    || shared_queue_.empty()
	// 			    || exclusive_queue_.top() < shared_queue_.top())
	// 			{
	// 				exclusive_wake_up();
	// 			}
	// 			else
	// 			{
	// 				shared_wake_up();
	// 			}
	// 			return;
	// 		}
	// 		if (!shared_queue_.empty())
	// 		{
	// 			shared_wake_up();
	// 		}
	// 	}
	//
	// 	void lock_shared(const int priority = 0)
	// 	{
	// 		//e:x -> ec ? wait : s++
	// 		//e:나 -> s++
	// 		//e:다른 -> wait
	// 		const auto this_id = this_thread::get_id();
	// 		unique_lock lock(member_access_lock_);
	//
	// 		//이미 현재 스래드가 독점/공유 잠금을 가지고 있는 경우
	// 		if (exclusive_owner_ == this_id)
	// 		{
	// 			auto [iter, success] = shared_owners_.try_emplace(this_id, 1);
	// 			if (!success)
	// 			{
	// 				++iter->second;
	// 			}
	// 			return;
	// 		}
	// 		auto so_it = shared_owners_.find(this_id);
	// 		if (so_it != shared_owners_.end())
	// 		{
	// 			++so_it->second;
	// 			return;
	// 		}
	//
	// 		//경우의 수
	// 		//e:x -> s:x -> ec ? wait : s=1
	// 		//       s:다른 -> ec ? wait : s=1
	// 		//e:다른 -> s:x -> wait
	// 		//          s:다른 -> wait
	//
	// 		if (exclusive_owner_ == std::thread::id())
	// 		{
	// 			if (exclusive_queue_.empty())
	// 			{
	// 				shared_owners_.emplace(this_id, 1);
	// 				return;
	// 			}
	// 			auto now = std::chrono::steady_clock::now();
	// 			auto exclusive_condition = exclusive_maximum_wait_time_ <= now - exclusive_queue_.top()->time_;
	// 			if (!exclusive_condition)
	// 			{
	// 				shared_owners_.emplace(this_id, 1);
	// 				return;
	// 			}
	// 		}
	// 		auto _qd = new qd(priority, this_id);
	// 		shared_queue_.push(_qd);
	// 		lock.unlock();
	// 		_qd->flag_.wait(false);
	// 		delete _qd;
	// 	}
	//
	// 	void unlock_shared()
	// 	{
	// 		// e:x -> s:x -> 무시
	// 		//        s:나 -> s--
	// 		//        s:다른 -> 무시
	// 		//        s:all -> s--
	// 		// e:나 -> s:x -> 무시
	// 		//         s:나 -> s--
	// 		// e:다른 -> s:x -> 무시
	// 		//           s:다른 -> 무시
	// 		const auto this_id = this_thread::get_id();
	// 		unique_lock lock(member_access_lock_);
	// 		const auto so_it = shared_owners_.find(this_id);
	// 		if (so_it == shared_owners_.end() || 0 < --so_it->second)
	// 			return;
	// 		shared_owners_.erase(so_it);
	//
	// 		//현재 상태의 경우의 수
	// 		//e:x -> s:x -> ec ? e_notify : (e_q < s_q ? e_notify : s_notify)
	// 		//       s:다른 -> ec ? 무시 : s_notify
	// 		//e:나 -> s:x -> 무시
	// 		if (exclusive_owner_ == this_id)
	// 			return;
	//
	// 		//스래드 깨우기
	// 		if (!exclusive_queue_.empty())
	// 		{
	// 			auto now = std::chrono::steady_clock::now();
	// 			bool exclusive_condition = exclusive_maximum_wait_time_ <= now - exclusive_queue_.top()->time_;
	// 			if (exclusive_condition
	// 			    || shared_queue_.empty()
	// 			    || exclusive_queue_.top() < shared_queue_.top())
	// 			{
	// 				exclusive_wake_up();
	// 			}
	// 			else
	// 			{
	// 				shared_wake_up();
	// 			}
	// 			return;
	// 		}
	// 		if (!shared_queue_.empty())
	// 		{
	// 			shared_wake_up();
	// 		}
	// 	}
	// };
}

using namespace ns_test3;


#include <random>

void thread_lock_test()
{
	shared_recursive_timed_mutex_priority lock;

	auto func = [&lock](int id) {
		int exclusive_lock_count = 0;
		int shared_lock_count = 0;

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> lock_type_dist(0, 1); // 독점 락과 공유 락을 결정하는 확률

		for (long long i = 0; i < 100; ++i)
		{
			bool is_exclusive = lock_type_dist(gen) == 0;

			if (is_exclusive)
			{
				// 독점 락의 경우
				int action = (exclusive_lock_count == 0) ? 0 : gen() % (exclusive_lock_count + 1);
				if (action < exclusive_lock_count)
				{
					cout << to_string(id) + " unlock\n";
					--exclusive_lock_count;
					lock.unlock();
				}
				else
				{
					cout << to_string(id) + " lock\n";
					++exclusive_lock_count;
					lock.lock();
				}
			}
			else
			{
				// 공유 락의 경우
				int action = (shared_lock_count == 0) ? 0 : gen() % (shared_lock_count + 1);
				if (action < shared_lock_count)
				{
					cout << to_string(id) + " unlock_shared\n";
					--shared_lock_count;
					lock.unlock_shared();
				}
				else
				{
					cout << to_string(id) + " lock_shared\n";
					++shared_lock_count;
					lock.lock_shared();
				}
			}
		}

		while (0 < exclusive_lock_count)
		{
			--exclusive_lock_count;
			lock.unlock();
		}

		while (0 < shared_lock_count)
		{
			--shared_lock_count;
			lock.unlock_shared();
		}

		std::cout << "Thread " << id << " completed." << std::endl;
	};

	std::vector<std::thread> threads;

	for (int i = 0; i < 2; ++i)
	{
		threads.emplace_back(func, i);
	}

	for (auto &t: threads)
	{
		t.join();
	}

	std::cout << "All threads completed." << std::endl;
}


int lock_test()
{
	log::println("lock_test");
	thread_lock_test();
	return 1;

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

	//shared_recursive_timed_mutex_priority 테스트
	shared_recursive_timed_mutex_priority sr;
	sr.lock();
	// std::this_thread::sleep_for(std::chrono::milliseconds(100));
	t = thread([&] {
		sr.lock_shared();
		cout << "thread start" << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		cout << "thread end" << endl;
		sr.unlock_shared();
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	sr.lock_shared();
	sr.lock_shared();
	sr.lock_shared();
	sr.lock_shared();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	cout << "unlock" << endl;
	sr.unlock();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cout << "unlock_shared" << endl;
	sr.unlock_shared();
	sr.unlock_shared();
	sr.unlock_shared();
	cout << "main: lock()\n";
	sr.lock();
	cout << "main: locking\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	sr.unlock_shared();
	sr.unlock();

	t.join();
	return 0;
}
