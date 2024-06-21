// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

namespace uniq::hierarchy
{
	template<typename T>
	template<hierarchy_feature::callback_mode mode>
	void hierarchy_feature::chain<T>::callback_call_bfs(const T &t, const bool find_root)
	{
		std::queue<chain*> queue; //BFS(너비 우선 탐색)를 위한 큐
		auto root = this;
		if (find_root)
		{
			while (root->parent_ && root->sync_) //최상위 노드부터 시작
			{
				root = root->parent_;
			}
		}
		queue.push(root);
		while (!queue.empty())
		{
			chain* current = queue.front();
			queue.pop();
			auto it = current->callback_list_.find(mode);
			if (it != current->callback_list_.end())
			{
				for (const auto &[id, callback] : it->second)
				{
					callback(t);
				}
			}
			for (auto child : current->children_)
			{
				if (child->sync_)
				{
					queue.push(child);
				}
			}
		}
	}

	template<typename T>
	void hierarchy_feature::chain<T>::sync_refresh(const bool sync)
	{
		if (!parent_) return;
		auto change_value_flag = false;
		if (sync && *parent_->value_ != *value_)
		{
			change_value_flag = true;
			callback_call_bfs<callback_mode::change_before>(*value_, false);
		}
		std::stack<chain*> stack; //DFS(깊이 우선 탐색)를 위한 스택
		stack.push(this);
		std::shared_ptr<T> target = sync ? parent_->value_ : std::make_shared<T>(*parent_->value_);
		while (!stack.empty())
		{
			chain* current = stack.top();
			stack.pop();
			current->value_ = target;
			for (auto child : current->children_)
			{
				if (child->sync_)
				{
					stack.push(child);
				}
			}
		}
		if (change_value_flag)
		{
			callback_call_bfs<callback_mode::change_after>(*value_, false);
		}
	}


	template<typename T>
	hierarchy_feature::chain<T>::chain(const hierarchy_feature* self, const T& value, const bool sync):
		self_(self), value_(std::make_shared<T>(value)), sync_(sync), parent_(nullptr)
	{
	}

	template<typename T>
	hierarchy_feature::chain<T>::~chain()
	{
		auto it = callback_list_.find(callback_mode::remove_before);
		if (it != callback_list_.end())
		{
			for (const auto &[id, callback] : it->second)
			{
				callback(*value_);
			}
		}
		callback_list_.clear();
		if (parent_)
		{
			parent_->children_.erase(this);
		}
		for (auto child : children_)
		{
			if (!child->parent_) continue;
			if (child->sync_)
				child->sync_refresh(false); //자식 노드의 value_를 분리(독립)시킴
			child->parent_ = nullptr;
		}
	}

	template<typename T>
	hierarchy_feature::chain<T>::operator T&() const
	{
		return *value_;
	}

	template<typename T>
	hierarchy_feature::chain<T>& hierarchy_feature::chain<T>::operator=(const T& value)
	{
		set(value);
		return *this;
	}

	template<typename T>
	const T& hierarchy_feature::chain<T>::operator*() const
	{
		return *value_;
	}

	template<typename T>
	T * hierarchy_feature::chain<T>::operator->()
	{
		return value_.get();
	}

	template<typename T>
	bool hierarchy_feature::chain<T>::sync_get() const
	{
		return sync_;
	}

	template<typename T>
	void hierarchy_feature::chain<T>::sync_set(const bool sync)
	{
		if (sync_ == sync) return;
		sync_refresh(sync);
		sync_ = sync;
	}

	template<typename T>
	const T& hierarchy_feature::chain<T>::get() const
	{
		return *value_;
	}

	template<typename T>
	bool hierarchy_feature::chain<T>::set(const T& value)
	{
		if (*value_ == value) return false;
		callback_call_bfs<callback_mode::change_before>(value);
		*value_ = value;
		callback_call_bfs<callback_mode::change_after>(value);
		return true;
	}

	template<typename T>
	bool hierarchy_feature::chain<T>::parent_set(chain *parent)
	{
		if (!parent || parent == this)
		{
			log::error("부모 노드가 존재하지 않거나 자기 자신을 부모 노드로 설정할 수 없습니다.");
			return false;
		}
		if (parent_)
		{
			log::error("부모 노드가 이미 존재합니다.(변경이 필요한 경우 명시적으로 parent_remove() 함수를 호출해야 합니다.)");
			return false;
		}
		if (parent == parent_) return false;
		//TODO: 순환 참조 검사
		parent_ = parent;
		parent_->children_.insert(this);
		if (sync_) sync_refresh(true);
		return true;
	}

	template<typename T>
	bool hierarchy_feature::chain<T>::parent_remove()
	{
		if (!parent_) return false;
		if (sync_) sync_refresh(false);
		parent_->children_.erase(this);
		parent_ = nullptr;
		callback_list_.clear();
		return true;
	}

	template<typename T>
	bool hierarchy_feature::chain<T>::child_add(chain *child)
	{
		if (!child || child == this)
		{
			log::error("자식 노드가 존재하지 않거나 자기 자신을 자식 노드로 설정할 수 없습니다.");
			return false;
		}
		if (child->parent_ == this) return false;
		if (child->parent_)
		{
			log::error("자식 노드가 이미 다른 부모 노드를 가지고 있습니다.(변경이 필요한 경우 명시적으로 parent_remove() 함수를 호출해야 합니다.)");
			return false;
		}
		return child->parent_set(this);
	}

	template<typename T>
	bool hierarchy_feature::chain<T>::child_remove(chain *child)
	{
		if (!child || child == this) return false;
		if (child->parent_ != this) return false;
		return child->parent_remove();
	}

	template<typename T>
	template<hierarchy_feature::callback_mode mode>
	int hierarchy_feature::chain<T>::callback_add(std::function<void(const T &)> callback)
	{
		// if (!parent_)
		// {
		// 	log::error("부모 노드가 존재하지 않습니다.");
		// 	return 0;
		// }
		callback_list_[mode].emplace(++callback_id_, callback);
		return callback_id_;
	}
	template<typename T>
	bool hierarchy_feature::chain<T>::callback_remove(int callback_id)
	{
		// if (!parent_) return false;
		for (auto &it : callback_list_)
		{
			auto it2 = it.second.find(callback_id);
			if (it2 != it.second.end())
			{
				it.second.erase(it2);
				return true;
			}
		}
		return false;
	}

	template<typename T>
	template<hierarchy_feature::callback_mode mode>
	bool hierarchy_feature::chain<T>::callback_remove(int callback_id)
	{
		// if (!parent_) return false;
		auto it = callback_list_.find(mode);
		if (it == callback_list_.end()) return false;
		auto it2 = it->second.find(callback_id);
		if (it2 == it->second.end()) return false;
		it->second.erase(it2);
		if (it->second.empty()) callback_list_.erase(it);
		return true;
	}
}
