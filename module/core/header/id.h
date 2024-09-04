// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include <any>

#include "lock.h"
#include "log.h"
#include "api_preset.h"

namespace uniq::core
{
	//ID_manager는 ID<T>를 상속받은 객체를 관리합니다.
	class ID_manager final
	{
		template<typename T> friend class ID;
		static std::unordered_map<id_t, std::any> registry_;
		static id_t id_;
		static spin_lock lock_;

		// struct callback_data
		// {
		// 	id_t id;
		// };

		static id_t generate_ID();
		template<typename T>
		static void register_ID(const id_t id, std::shared_ptr<T> obj)
		{
			std::unique_lock lock(lock_);
			registry_[id] = std::weak_ptr<T>(obj);
		}
		static void unregister_ID(id_t id);
	public:
		template<typename T>
		static std::optional<std::shared_ptr<T>> get_shared_ptr_o(const id_t id)
		{
			if (id < static_cast<id_t>(api::predefined_ID::last))
			{
				log::error("ID " + std::to_string(id) + " is a predefined ID.");
				return std::nullopt;
			}
			std::unique_lock lock(lock_);
			const auto it = registry_.find(id);
			if (it == registry_.end())
			{
				log::error("ID " + std::to_string(id) + " is not registered."
					+ "Please check if the object is created by "+ typeid(T).name() + "::create().");
				return std::nullopt;
			}
			if (auto& second = it->second; second.has_value())
			{
				if (second.type() == typeid(std::weak_ptr<T>)) //bad_any_cast 방지
				{
					return std::any_cast<std::weak_ptr<T>>(second).lock();
				}
			}
			log::error("ID " + std::to_string(id) + " is not registered."
				+ "Please check if the object is created by "+ typeid(T).name() + "::create().");
			return std::nullopt;
		}

		template<typename T>
		static std::shared_ptr<T> get_shared_ptr(const id_t id)
		{
			auto _o = get_shared_ptr_o<T>(id);
			if (!_o) return nullptr;
			return _o.value();
		}
	};

	//public ID<T> 상속을 통해 ID_manager에 ID를 생성하고 등록하는 클래스를 만듭니다.
	//부여 받은 ID는 ID_manager를 통해 해당 객체를 참조할 수 있습니다.
	//해당 클래스는 shared_ptr를 위해 ID<T>::create()를 통해 객체를 생성하도록 강제하므로,
	//객체가 임의로 생성되지 않도록 생성자를 private로 선언하는 것을 권장합니다.
	template<typename T> class ID
	{
	public:
		//TODO: create()를 경유하지 않은 객체를 생성할 수 없게 함.
	private:
		id_t id_ = 0; //api::predefined_ID::last보다 작은 값은 사용하지 않습니다.
		id_t workspace_id_ = workspace_preset::workspace_info.get_id();
	protected:
		ID() : id_(ID_manager::generate_ID()) {}
		~ID()
		{
			ID_manager::unregister_ID(id_);
			#ifdef UNIQ_DLL_API
			// ID를 상속받은 클래스가 소멸될 때마다 부여받은 ID를 콜백 매니저에 알립니다.
			api::callback_manager.add_destroy_ID(id_);
			#endif
		}
	public:
		template <typename... K>
		static std::shared_ptr<T> create(K &&...args)
		{
			//make_shared에 프라이빗 생성자를 사용하기 위한 구조체
			//컴파일 최적화로 MakeSharedEnabler의 오버 헤드는 없음.
			struct make_shared_enabler : T
			{
				explicit make_shared_enabler(K &&...args) : T(std::forward<K>(args)...) {}
			};
			std::shared_ptr<T> sp = std::make_shared<make_shared_enabler>(std::forward<K>(args)...);

			ID_manager::register_ID(sp->id_, sp);
			#ifdef UNIQ_DLL_API
			// ID를 상속받은 클래스가 생성될 때마다 부여받은 ID를 콜백 매니저에 알립니다.
			api::callback_manager.add_create_ID(sp->id_);
			#endif
			return sp;
		}
	public:
		ID(const ID&) = delete;
		ID& operator=(const ID&) = delete;
		ID(ID&&) = delete;
		ID& operator=(ID&&) = delete;
		[[nodiscard]] id_t ID_get() const
		{
			return id_;
		}
		[[nodiscard]] id_t workspace_ID_get() const
		{
			return workspace_id_;
		}
	};
}
