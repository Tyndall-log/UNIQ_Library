// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#ifdef ANDROID
	#define JUCE_CORE_INCLUDE_JNI_HELPERS 1
	#include <android/log.h>
#endif
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
//#include <juce_audio_basics/juce_audio_basics.h>
//#include <juce_audio_devices/juce_audio_devices.h>
//#include <juce_audio_formats/juce_audio_formats.h>
//#include <juce_audio_processors/juce_audio_processors.h>
//#include <juce_gui_basics/components/juce_Component.h>

#include "core.hpp"
#include <algorithm>
#include <iostream>
#include <ranges>
#include <unordered_map>
#include <any>
#include <optional>
#include <typeindex>
#include <memory>
#include <format>


namespace uniq
{
#ifdef _WIN32
#define API extern "C" __declspec(dllexport)
#else
#define API extern "C" __attribute__((visibility("default")))
#endif
	
	class log
	{
	public:
		static std::string message_temp;
		static std::string message;
		
		static void print(std::string_view str)
		{
			message += str;
			#ifdef ANDROID
			__android_log_print(ANDROID_LOG_INFO, "uniq", "%s", str.data());
			#else
			std::cout << str;
			#endif
		}
		
		static void println(std::string_view str)
		{
			print(str);
			message += "\n";
			#ifndef ANDROID
			std::cout << std::endl;
			#endif
			
		}
		
		static std::string& get()
		{
			message_temp = message;
			message.clear();
			return message_temp;
		}
	};
	
	
	//ID_manager는 ID<T>를 상속받은 객체를 관리합니다.
	class ID_manager final
	{
		template<typename T> friend class ID;
		static std::unordered_map<std::size_t, std::any> registry_;
		//static std::unordered_map<std::size_t, std::any> memory
		static std::size_t id_; //0은 무효한 ID입니다.
		static juce::SpinLock lock_;
	private:
		static std::size_t generate_ID();
		template<typename T>
		static void register_ID(std::size_t id, std::shared_ptr<T> obj)
		{
			juce::SpinLock::ScopedLockType scoped_lock(lock_);
			registry_[id] = std::weak_ptr<T>(obj);
		}
		static void unregister_ID(std::size_t id);
	public:
		template<typename T>
		static std::optional<std::shared_ptr<T>> get_shared_ptr_by_ID(std::size_t id)
		{
			juce::SpinLock::ScopedLockType scoped_lock(lock_);
			if (id == 0) return std::nullopt;
			auto it = registry_.find(id);
			if (it == registry_.end()) return std::nullopt;
			if (auto& second = it->second; second.has_value())
			{
				if (second.type() == typeid(std::weak_ptr<T>)) //bad_any_cast 방지
				{
					return std::any_cast<std::weak_ptr<T>>(second).lock();
				}
			}
			else
			{
				log::println("ID_manager::get_shared_ptr_by_ID: ID " + std::to_string(id) + " is not registered."
							 + "Please check if the object is created by "+ typeid(T).name() + "::create().");
				throw std::runtime_error("ID_manager::get_shared_ptr_by_ID: ID " + std::to_string(id) + " is not registered."
										 + "Please check if the object is created by "+ typeid(T).name() + "::create().");
			}
			return std::nullopt;
		}
	};
	
	//public ID<T> 상속을 통해 ID_manager에 ID를 생성하고 등록하는 클래스를 만듭니다.
	//부여 받은 ID는 ID_manager를 통해 해당 객체를 참조할 수 있습니다.
	//해당 클래스는 shared_ptr를 위해 ID<T>::create()를 통해 객체를 생성하도록 강제하므로,
	//객체가 임의로 생성되지 않도록 생성자를 private로 선언하는 것을 권장합니다.
	template<typename T> class ID
	{
		//TODO: create()를 경유하지 않은 객체를 생성할 수 없게 함.
	private:
		std::size_t id_ = 0; //0은 무효한 ID입니다.
	protected:
		ID() : id_(ID_manager::generate_ID()) {};
		~ID()
		{
			ID_manager::unregister_ID(id_);
		}
	public:
		template <typename... K>
		static std::shared_ptr<T> create(K &&...args)
		{
			//make_shared에 프라이빗 생성자를 사용하기 위한 구조체
			//컴파일 최적화로 MakeSharedEnabler의 오버 헤드는 없음.
			struct make_shared_enabler : public T
			{
				explicit make_shared_enabler(K &&...args) : T(std::forward<K>(args)...) {}
			};
			std::shared_ptr<T> sp = std::make_shared<make_shared_enabler>(std::forward<K>(args)...);
			
			ID_manager::register_ID(sp->id_, sp);
			return sp;
		}
	public:
		ID(const ID&) = delete;
		ID& operator=(const ID&) = delete;
		ID(ID&&) = delete;
		ID& operator=(ID&&) = delete;
		[[nodiscard]] size_t ID_get() const
		{
			return id_;
		}
	};
	
	class hierarchy
	{
	private:
		enum class mode : std::uint8_t { add, remove };
		static std::size_t relationship_id_; //0은 무효한 ID입니다.
		std::vector<std::function<void(std::any, mode)>> chain_func_list_;
		std::vector<hierarchy*> child_list_;
		std::vector<hierarchy*> parent_list_;
	protected:
		template<typename T>
		class chain
		{
			struct chain_id
			{
				std::size_t id;
				chain<T>* ptr;
			};
			
			struct child_data
			{
				std::function<void(std::any)> child_add;
				std::map<hierarchy*, chain<T>*> child_map;
				
				void child_remove(hierarchy* class_ptr)
				{
					if (auto it = child_map.find(class_ptr); it != child_map.end())
					{
						it->second->parent_remove(class_ptr);
						child_map.erase(it);
					}
				}
			};
			struct parent_data
			{
				std::map<hierarchy*, chain_id*> parent_map; //검색용
				std::vector<chain_id*> parent_list;
				
				~parent_data()
				{
					for (auto& [key, value]: parent_map)
					{
						delete value;
					}
				}
				
				void parent_add(hierarchy* class_ptr, chain<T>* chain_ptr, std::size_t id)
				{
					if (auto [iter, inserted] = parent_map.try_emplace(class_ptr, new chain_id{id, chain_ptr}); inserted)
					{
						parent_list.emplace_back(iter->second);
					}
				}
				
				void parent_remove(hierarchy* class_ptr)
				{
					if (auto it = parent_map.find(class_ptr); it != parent_map.end())
					{
						auto id = it->second->id;
						parent_map.erase(it);
						auto iter = std::lower_bound(parent_list.begin(), parent_list.end(), id,
													 [](const chain_id* a, std::size_t b) { return a->id < b; });
						if (iter != parent_list.end())
						{
							//속도를 위해 지연 삭제 구현 필요
							//*iter = nullptr;
							parent_list.erase(iter);
						}
						delete it->second;
					}
				}
			};
		private:
			//hierarchy* parent_;
			std::shared_ptr<T> value_;
			bool sync_{};
			std::map<std::type_index, child_data> child_map_; //child_map_[class_pointer_type] = child_data
			parent_data parent_list_;
			std::vector<parent_data*> parent_priority_list_; //인덱스가 높을수록 우선순위가 높습니다.
			std::map<std::type_index, parent_data*> parent_priority_map_;
			
			template<typename K>
			void sync_type_register(chain<T> K::* member_ptr)
			{
				if (auto [iter, inserted] = child_map_.try_emplace(typeid(K*)); inserted)
				{
					auto& data = iter->second;
					data.child_add = [this, &data, member_ptr](std::any class_ptr_any)
					{
						auto class_ptr = std::any_cast<K*>(class_ptr_any);
						auto chain_ptr = &(class_ptr->*member_ptr);
						auto cp_data = chain_ptr->value_;
						data.child_map.try_emplace(class_ptr, chain_ptr);
						
						chain_ptr->parent_add(class_ptr, this, ++relationship_id_);
					};
				}
			}
			
			void parent_add(hierarchy* class_ptr, chain<T>* chain_ptr, std::size_t id)
			{
				if (auto it = parent_priority_map_.find(typeid(class_ptr)); it != parent_priority_map_.end())
				{
					auto* pd = it->second;
					pd->parent_add(class_ptr, chain_ptr, id);
				}
				else
				{
					parent_list_.parent_add(class_ptr, chain_ptr, id);
				}
				
				if (sync_)
				{
					auto parent = parent_priority_get();
					if (parent->value_ != value_)
					{
						sync_bfs(value_, parent->value_);
					}
				}
			}
			
			void parent_remove(hierarchy* class_ptr)
			{
				if (auto it = parent_priority_map_.find(typeid(class_ptr)); it != parent_priority_map_.end())
				{
					auto* pd = it->second;
					pd->parent_remove(class_ptr);
				}
				else
				{
					parent_list_.parent_remove(class_ptr);
				}
			}
			
			chain<T>* parent_priority_get()
			{
				if (parent_priority_list_.empty())
				{
					if (parent_list_.parent_list.empty()) return nullptr;
					return parent_list_.parent_list.back()->ptr; //맨 마지막에 추가된 부모를 반환합니다.
				}
				return parent_priority_list_.back()->parent_list.back()->ptr;
			}
			
			void sync_bfs(std::shared_ptr<T>& prev_value, std::shared_ptr<T>& change_value)
			{
				auto queue = std::queue<chain<T>*>(); //BFS
				queue.emplace(this);
				while (!queue.empty())
				{
					auto* chain_ptr = queue.front();
					queue.pop();
					if (chain_ptr->value_ != prev_value) continue;
					chain_ptr->value_ = change_value;
					for (auto& [key, value]: chain_ptr->child_map_)
					{
						for (auto& [key2, value2]: value.child_map)
						{
							queue.emplace(value2);
						}
					}
				}
			}
			//protected:
		
		public:
			chain(hierarchy* parent, T value) : chain(parent, value, false){};
			template<typename... Others>
			requires (... && std::is_member_pointer_v<Others>) // Others는 chain<T> K::* 형식의 멤버 포인터여야 합니다.
			chain(hierarchy* parent, T value, bool sync, Others... others)
			{
				//parent_ = parent;
				value_ = std::make_shared<T>(value);
				sync_ = sync;
				
				// 외부에 등록
				if (parent)
				{
					parent->chain_func_list_.emplace_back([&](std::any class_ptr, mode mode)
					{
						switch (mode)
						{
							case mode::add: // 추가 모드
								if (auto it = child_map_.find(class_ptr.type()); it != child_map_.end())
									it->second.child_add(class_ptr);
								break;
							case mode::remove: // 삭제 모드
								if (auto it = child_map_.find(class_ptr.type()); it != child_map_.end())
									it->second.child_remove(std::any_cast<hierarchy*>(class_ptr));
								break;
							default:
								break;
						}
					});
				}
				
				// sync_type_register 호출
				(sync_type_register(others), ...);
			}
			
			void sync_set(bool sync)
			{
				if (sync_ == sync) return;
				sync_ = sync;
				auto parent = parent_priority_get();
				if (!parent) return;
				
				if (sync)
				{
					sync_bfs(value_, parent->value_);
				}
				else
				{
					auto new_value = std::make_shared<T>(*value_);
					sync_bfs(value_, new_value);
				}
			}
			
			T value_get()
			{
				return *value_;
			}
			
			void value_set(T value)
			{
				*value_ = value;
			}

//			chain& operator=(const T& value)
//			{
//				value_ = value;
//				return *this;
//			}
//
//			operator T()
//			{
//				return *value_;
//			}
		};
	
	public:
		~hierarchy();
		
		template<typename T>
		requires std::is_base_of_v<hierarchy, T> // T는 hierarchy를 상속받아야 합니다.
		inline void child_add(std::shared_ptr<T>& child)
		{
			child_add(child.get());
		}
		
		template<typename T>
		requires std::is_base_of_v<hierarchy, T> // T는 hierarchy를 상속받아야 합니다.
		inline void child_remove(std::shared_ptr<T>& child)
		{
			child_remove(child.get());
		}
		
		template<typename T>
		requires std::is_base_of_v<hierarchy, T> // T는 hierarchy를 상속받아야 합니다.
		inline void parent_add(std::shared_ptr<T>& parent)
		{
			parent->child_add(this);
		}
		
		template<typename T>
		requires std::is_base_of_v<hierarchy, T> // T는 hierarchy를 상속받아야 합니다.
		inline void parent_remove(std::shared_ptr<T>& parent)
		{
			parent->child_remove(this);
		}
	
	private:
		template<typename T>
		requires std::is_base_of_v<hierarchy, T> // T는 hierarchy를 상속받아야 합니다.
		void child_add(T* child)
		{
			child_list_.emplace_back(child);
			child->parent_list_.emplace_back(this);
			for (auto& f: chain_func_list_)
			{
				f(child, mode::add);
			}
		}
		
		template<typename T>
		requires std::is_base_of_v<hierarchy, T> // T는 hierarchy를 상속받아야 합니다.
		void child_remove(T* child)
		{
			for (auto& f: chain_func_list_)
			{
				f(static_cast<hierarchy*>(child), mode::remove);
			}
		}
	};
	
	
	//콘솔에서 메인 스레드와 독립적으로 메시지 이벤트 처리할 수 있도록 하는 클래스
	class MainMessageThread : public juce::Thread
	{
	public:
		MainMessageThread();
		~MainMessageThread() override;
		
		void run() override;
	};
}
