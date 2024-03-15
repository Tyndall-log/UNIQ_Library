// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "lock.h"

using namespace std;

namespace uniq
{
#pragma region spin_lock

	void spin_lock::lock()
	{
		while (flag_.test_and_set(memory_order_acquire))
		{
			while (flag_.test(memory_order_relaxed));
			// this_thread::yield();
		}
	}

	bool spin_lock::try_lock() noexcept
	{
		return !flag_.test_and_set(memory_order_acquire);
	}

	void spin_lock::unlock()
	{
		flag_.clear(memory_order_release);
	}

#pragma endregion spin_lock

#pragma region shared_recursive_timed_mutex_priority

	strong_ordering shared_recursive_timed_mutex_priority::qd::operator<=>(const qd &other) const
	{
		if (const auto result = priority_ <=> other.priority_; result != 0)
			return result;
		return time_ <=> other.time_;
	}

	bool shared_recursive_timed_mutex_priority::qd_compare::operator()(const qd *lhs, const qd *rhs) const
	{
		if (lhs->priority_ != rhs->priority_)
			return lhs->priority_ > rhs->priority_;
		return lhs->time_ > rhs->time_;
	}

	bool shared_recursive_timed_mutex_priority::exclusive_try_lock(const int priority)
	{
		//e:x -> s:x -> e=1, true
		//       s:나 -> e=1, true
		//       s:다른 -> false
		//       s:all -> false
		//e:나 -> e++, true
		//e:다른 -> false
		const auto this_id = this_thread::get_id();
		if (exclusive_owner_ == this_id)
		{
			++exclusive_count_;
			return true;
		}
		if (exclusive_owner_ == thread::id())
		{
			if (shared_owners_.empty() || (shared_owners_.size() == 1 && shared_owners_.contains(this_id)))
			{
				exclusive_owner_ = this_id;
				exclusive_count_ = 1;
				return true;
			}
		}
		return false;
	}

	bool shared_recursive_timed_mutex_priority::shared_try_lock(const int priority)
	{
		//e:x -> ec ? wait : s++
		//e:나 -> s++
		//e:다른 -> wait
		const auto this_id = this_thread::get_id();

		//이미 현재 스래드가 독점/공유 잠금을 가지고 있는 경우
		if (exclusive_owner_ == this_id)
		{
			auto [iter, success] = shared_owners_.try_emplace(this_id, 1);
			if (!success)
			{
				++iter->second;
			}
			return true;
		}
		auto so_it = shared_owners_.find(this_id);
		if (so_it != shared_owners_.end())
		{
			++so_it->second;
			return true;
		}

		//경우의 수
		//e:x -> s:x -> ec ? wait : s=1
		//       s:다른 -> ec ? wait : s=1
		//e:다른 -> s:x -> wait
		//          s:다른 -> wait

		if (exclusive_owner_ == thread::id())
		{
			if (exclusive_queue_.empty())
			{
				shared_owners_.emplace(this_id, 1);
				return true;
			}
			auto now = chrono::steady_clock::now();
			auto exclusive_condition = exclusive_maximum_wait_time_ <= now - exclusive_queue_.top()->time_;
			if (!exclusive_condition)
			{
				shared_owners_.emplace(this_id, 1);
				return true;
			}
		}
		return false;
	}

	void shared_recursive_timed_mutex_priority::exclusive_wake_up()
	{
		auto &_qd = exclusive_queue_.top();
		exclusive_owner_ = _qd->id_;
		exclusive_count_ = 1;
		_qd->flag_.test_and_set(memory_order_release);
		_qd->flag_.notify_one();
		exclusive_queue_.pop();
	}

	void shared_recursive_timed_mutex_priority::shared_wake_up()
	{
		while (!shared_queue_.empty())
		{
			auto &_qd = shared_queue_.top();
			const auto [iter, success] = shared_owners_.try_emplace(_qd->id_, 1);
			if (!success) ++iter->second;
			_qd->flag_.test_and_set(memory_order_release);
			_qd->flag_.notify_one();
			shared_queue_.pop();
		}
	}

	shared_recursive_timed_mutex_priority::~shared_recursive_timed_mutex_priority()
	{
		unique_lock lock(member_access_lock_);
		while (!exclusive_queue_.empty())
		{
			delete exclusive_queue_.top();
			exclusive_queue_.pop();
		}
		while (!shared_queue_.empty())
		{
			delete shared_queue_.top();
			shared_queue_.pop();
		}
	}

	// 경우의 수 정리
	// e는 exclusive, s는 shared
	// x는 없음, 나는 현재 스레드, 다른은 다른 스레드, all은 현재 스레드와 다른 스레드
	// e:x -> s:x -> 가능
	//        s:나 -> 가능
	//        s:다른 -> 가능
	//        s:all -> 가능
	// e:나 -> s:x -> 가능
	//         s:나 -> 가능
	//         s:다른 -> 불가능
	//         s:all -> 불가능
	// e:다른 -> s:x -> 가능
	//           s:나 -> 불가능
	//           s:다른 -> 가능
	//           s:all -> 불가능

	void shared_recursive_timed_mutex_priority::lock(const int priority)
	{
		unique_lock lock(member_access_lock_);
		if (exclusive_try_lock(priority))
			return;
		const auto this_id = this_thread::get_id();
		auto _qd = new qd(priority, this_id);
		exclusive_queue_.push(_qd);
		lock.unlock();
		_qd->flag_.wait(false);
		delete _qd;
	}

	bool shared_recursive_timed_mutex_priority::try_lock(const int priority) noexcept
	{
		unique_lock lock(member_access_lock_);
		return exclusive_try_lock(priority);
	}

	void shared_recursive_timed_mutex_priority::unlock()
	{
		//e:x -> 무시
		//e:나 -> s:x -> e--
		//        s:나 -> e--
		//e:다른 -> 무시
		const auto this_id = this_thread::get_id();
		unique_lock lock(member_access_lock_);
		if (exclusive_owner_ != this_id || 0 < --exclusive_count_)
			return;

		exclusive_owner_ = thread::id();
		exclusive_count_ = 0;

		//현재 상태의 경우의 수
		//e:x -> s:x -> ec ? e_notify : (e_q < s_q ? e_notify : s_notify)
		//       s:나 -> 무시

		if (shared_owners_.contains(this_id))
		{
			shared_wake_up();
			return;
		}

		//스래드 깨우기
		if (!exclusive_queue_.empty())
		{
			auto now = chrono::steady_clock::now();
			bool exclusive_condition = exclusive_maximum_wait_time_ <= now - exclusive_queue_.top()->time_;
			if (exclusive_condition
			    || shared_queue_.empty()
			    || exclusive_queue_.top() < shared_queue_.top())
			{
				exclusive_wake_up();
			}
			else
			{
				shared_wake_up();
			}
			return;
		}
		if (!shared_queue_.empty())
		{
			shared_wake_up();
		}
	}

	void shared_recursive_timed_mutex_priority::lock_shared(const int priority)
	{
		unique_lock lock(member_access_lock_);
		if (shared_try_lock(priority))
			return;
		const auto this_id = this_thread::get_id();
		auto _qd = new qd(priority, this_id);
		shared_queue_.push(_qd);
		lock.unlock();
		_qd->flag_.wait(false);
		delete _qd;
	}

	bool shared_recursive_timed_mutex_priority::try_lock_shared(const int priority) noexcept
	{
		unique_lock lock(member_access_lock_);
		return shared_try_lock(priority);
	}

	void shared_recursive_timed_mutex_priority::unlock_shared()
	{
		// e:x -> s:x -> 무시
		//        s:나 -> s--
		//        s:다른 -> 무시
		//        s:all -> s--
		// e:나 -> s:x -> 무시
		//         s:나 -> s--
		// e:다른 -> s:x -> 무시
		//           s:다른 -> 무시
		const auto this_id = this_thread::get_id();
		unique_lock lock(member_access_lock_);
		const auto so_it = shared_owners_.find(this_id);
		if (so_it == shared_owners_.end() || 0 < --so_it->second)
			return;
		shared_owners_.erase(so_it);

		//현재 상태의 경우의 수
		//e:x -> s:x -> ec ? e_notify : (e_q < s_q ? e_notify : s_notify)
		//       s:다른 -> ec ? 무시 : s_notify
		//e:나 -> s:x -> 무시
		if (exclusive_owner_ == this_id)
			return;

		//스래드 깨우기
		if (!exclusive_queue_.empty())
		{
			auto now = chrono::steady_clock::now();
			bool exclusive_condition = exclusive_maximum_wait_time_ <= now - exclusive_queue_.top()->time_;
			if (exclusive_condition
			    || shared_queue_.empty()
			    || exclusive_queue_.top() < shared_queue_.top())
			{
				exclusive_wake_up();
			}
			else
			{
				shared_wake_up();
			}
			return;
		}
		if (!shared_queue_.empty())
		{
			shared_wake_up();
		}
	}
#pragma endregion shared_recursive_timed_mutex_priority

}