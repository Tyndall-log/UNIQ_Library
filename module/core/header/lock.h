// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include <thread>
#include <atomic>
#include <chrono>
#include <queue>
#include <unordered_map>

namespace uniq
{
	class spin_lock
	{
		std::atomic_flag flag_ = {};
	public:
		spin_lock() = default;
		~spin_lock() = default;
		spin_lock(const spin_lock&) = delete;
		spin_lock& operator=(const spin_lock&) = delete;
		spin_lock(spin_lock&&) = delete;
		spin_lock& operator=(spin_lock&&) = delete;

		void lock();
		bool try_lock() noexcept;
		void unlock();
	};

	class shared_recursive_timed_mutex_priority
	{
	public:
		struct qd
		{
			int priority_ = 0;
			std::thread::id id_ = std::thread::id();
			std::chrono::time_point<std::chrono::steady_clock> time_ = std::chrono::steady_clock::now();
			std::atomic_flag flag_ = {};

			//우주선 연산자
			std::strong_ordering operator<=>(const qd& other) const;
		};

		struct qd_compare
		{
			bool operator()(const qd* lhs, const qd* rhs) const;
		};

		std::priority_queue<qd*, std::vector<qd*>, qd_compare> exclusive_queue_;
		std::priority_queue<qd*, std::vector<qd*>, qd_compare> shared_queue_;
		spin_lock member_access_lock_;
		std::thread::id exclusive_owner_;
		std::size_t exclusive_count_ = 0;
		std::unordered_map<std::thread::id, std::size_t> shared_owners_;
		const std::chrono::milliseconds exclusive_maximum_wait_time_ = std::chrono::milliseconds(1000);

	private:

		/// exclusive_try_lock는 member_access_lock_를 잠금한 상태에서 호출해야 한다.
		bool exclusive_try_lock(int priority);

		/// shared_try_lock는 member_access_lock_를 잠금한 상태에서 호출해야 한다.
		bool shared_try_lock(int priority);

		/// exclusive_wake_up는 member_access_lock_를 잠금한 상태에서 호출해야 한다.
		void exclusive_wake_up();

		/// shared_wake_up는 member_access_lock_를 잠금한 상태에서 호출해야 한다.
		void shared_wake_up();

	public:
		~shared_recursive_timed_mutex_priority();

		void lock(int priority = 0);

		bool try_lock(int priority = 0) noexcept;

		void unlock();

		void lock_shared(int priority = 0);

		bool try_lock_shared(int priority = 0) noexcept;

		void unlock_shared();
	};
}