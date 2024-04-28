// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include <memory>
#include <stack>
#include <queue>
#include <vector>
#include <set>
#include <map>
#include <any>
#include <variant>
#include <concepts>
#include <type_traits>
#include <functional>

#include "log.h"
// #include "event.h"

namespace uniq::hierarchy
{
	template<typename T>
	concept not_smart_pointer = !requires {
		requires std::is_same_v<T, std::unique_ptr<typename T::element_type>> ||
			std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
	};

	class hierarchy_feature
	{
	public:
		enum class callback_mode : std::uint8_t
		{
			change_before,
			change_after,
			remove_before,
			try_remove,
		};

	private:

	protected:
		template<typename T>
		class chain
		{
			static_assert(not_smart_pointer<T>,
				"내부적으로 스마트 포인터를 통해 T 객체가 관리되므로, T는 스마트 포인터가 될 수 없습니다.");
			const hierarchy_feature* self_;
			std::shared_ptr<T> value_;
			bool sync_; //sync_가 true이면 value_는 parent_의 value_를 참조하고, false이면 value_는 자체적인 값을 가집니다.
			chain* parent_;
			std::set<chain*> children_;
			std::map<callback_mode, std::map<int, std::function<void(const T&)>>> callback_list_;
			inline static int callback_id_ = 0;
			// void callback_call_bfs(const T &t, callback_mode mode);
			template<callback_mode mode>
			void callback_call_bfs(const T &t, bool find_root = true);
			void sync_refresh(bool sync);

		public:
			chain(const hierarchy_feature* self, const T& value, bool sync = false);
			~chain();
			[[nodiscard]] explicit operator T&() const;
			chain &operator=(const T &value);
			const T &operator*() const;
			T *operator->();
			[[nodiscard]] bool sync_get() const;
			void sync_set(bool sync);
			[[nodiscard]] const T &get() const;
			bool set(const T &value);
			// [[nodiscard]] chain *parent_get() const;
			bool parent_set(chain *parent);
			bool parent_remove();
			bool child_add(chain *child);
			bool child_remove(chain *child);
			template<callback_mode mode>
			[[nodiscard]]
			int callback_add(std::function<void(const T &)> callback);
			bool callback_remove(int callback_id);
			template<callback_mode mode>
			bool callback_remove(int callback_id);
		};

		// void hierarchy_init();
		// void hierarchy_update();
		// void hierarchy_destroy();
		// virtual ~hierarchy_feature();
	};
}
#include "hierarchy.hpp"