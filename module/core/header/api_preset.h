// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

// #include "id.h"
#include "log.h"
#include "alias.h"
#include "hash.h"


#include "workspace_preset.h"

namespace uniq::core::api
{
	enum class predefined_ID : id_t
	{
		unknown, // 0은 사용하지 않습니다.
		#ifdef UNIQ_DLL_API
		create, // 새로운 ID 객체가 생성될 때 사용합니다.
		destroy, // 기존 ID 객체가 소멸될 때 사용합니다.
		launchpad_manager, // launchpad_manager의 ID입니다.
		#endif
		last, // ID_manager에서 해당 값부터 ID를 생성합니다.
	};
}
#ifdef UNIQ_DLL_API

#ifdef _WIN32
#define API extern "C" __declspec(dllexport)
#else
#define API extern "C" __attribute__((visibility("default")))
#endif

namespace uniq::core::api
{
	/// global_api_lock은 API 함수들이 동시에 호출되는 것을 막기 위한 mutex입니다.<br>
	/// 기본적으로 모든 API 함수는 비동기 호출을 가정합니다.<br>
	/// 만일 API 함수에서 비동기를 호출을 지원한다면, global_api_lock을 꼭 사용할 필요는 없습니다.<br>
	/// 비동기 호출을 지원하지 않는 경우에만, global_api_lock을 사용하여 동시에 호출되는 상황을 방지해야 합니다.
	extern std::mutex global_api_lock;

	// struct workspace_info_raii
	// {
	// 	bool failed = false;
	// 	explicit workspace_info_raii(id_t id, std::source_location location = std::source_location::current());
	// 	[[nodiscard]] bool is_failed() const;
	//
	// 	~workspace_info_raii();
	// };

	struct API_callback_message_base
	{
		inline static const auto default_name = "unknown";
		const id_t api_workspace_id = workspace_preset::workspace_info.get_id();
		const char* api_func_name = workspace_preset::workspace_info.get_function_name().data();
		const id_t obj_id;
		const char* obj_id_type_name = default_name;
		const id_t func_id;
		const char* func_id_name = default_name;
		const id_t data_size;
		void* const data_ptr;

		API_callback_message_base(const id_t obj_id, const id_t func_id, const id_t data_size, void* data_ptr)
			: obj_id(obj_id), func_id(func_id), data_size(data_size), data_ptr(data_ptr) {}
		virtual ~API_callback_message_base() = default;
	};

	template<typename T>
	class API_callback_message final : public API_callback_message_base
	{
	public:
		API_callback_message(const id_t obj_id, const id_t func_id, const T* const & data)
			: API_callback_message_base(obj_id, func_id, sizeof(T), const_cast<T*>(data)) {}
		template<typename... Args>
		requires std::constructible_from<T, Args...>
		API_callback_message(const id_t obj_id, const id_t func_id, Args&&... args)
			: API_callback_message_base(obj_id, func_id, sizeof(T), new T(std::forward<Args>(args)...)) {}
		API_callback_message(const API_callback_message&) = delete;
		API_callback_message& operator=(const API_callback_message&) = delete;
		~API_callback_message() override {
			if (default_name != obj_id_type_name)
				delete obj_id_type_name;
			if (default_name != func_id_name)
				delete func_id_name;
			delete static_cast<T*>(data_ptr);
		}
		void set_obj_id_type_name(const char* type_name) { obj_id_type_name = strdup(type_name); }
		void set_func_name(const char* func_name) { func_id_name = strdup(func_name); }
	};

	class API_callback_manager
	{
		// std::queue<const API_callback_message_legacy * const> callback_queue;
		std::queue<API_callback_message_base*> callback_message_queue;
		spin_lock lock_;

		template<size_t N = 1024>
		struct ID_info
		{
			char type_name[N] = {};
			id_t type_hash = 0;

			consteval explicit ID_info(const std::source_location location = std::source_location::current())
			{
				#if defined(__clang__) || defined(__GNUC__)
				{
					constexpr std::string_view token = "T = ";
					std::string_view function_name = location.function_name();
					auto start = function_name.find(token);
					if (start == std::string_view::npos)
					{
						start = 0;
					}
					start += token.size();
					auto end = function_name.find_first_of(",];", start);
					if (end == std::string_view::npos)
					{
						end = function_name.size();
					}
					auto type_name = function_name.substr(start, end - start);
					std::copy(type_name.begin(), type_name.end(), this->type_name);
					this->type_name[type_name.size()] = '\0';
					type_hash = hash::fnv1a_hash(this->type_name);
				}
				#elif defined(_MSC_VER)
				{
					#warning "MSVC는 잘 작동하는지 테스트가 필요합니다."
					std::string_view function_name = location.function_name();
					auto start = function_name.find('<');
					if (start == std::string_view::npos)
					{
						start = 0;
					}
					start += 1;
					auto end = start;
					auto token_count = 1;
					while (token_count > 0 && end < function_name.size())
					{
						if (function_name[end] == '<')
						{
							++token_count;
						}
						else if (function_name[end] == '>')
						{
							--token_count;
						}
						++end;
					}
					--end;
					auto type_name = function_name.substr(start, end - start);
					std::copy(type_name.begin(), type_name.end(), this->type_name);
					this->type_name[type_name.size()] = '\0';
					type_hash = hash::fnv1a_hash(this->type_name);
				}
				#else
				{
					#warning "알려지지 않은 컴파일러이므로 추가 테스트가 필요합니다."
					std::string_view type_name = location.function_name();
					std::copy(type_name.begin(), type_name.end(), this->type_name);
					this->type_name[type_name.size()] = '\0';
					type_hash = hash::fnv1a_hash(this->type_name);
				}
				#endif
			}
		};

		template<size_t N = 1024>
		struct func_info_struct
		{
			char name[N] = {};
			id_t hash = 0;

			consteval explicit func_info_struct(const std::source_location location = std::source_location::current())
			{
				std::string_view type_name = location.function_name();
				std::copy(type_name.begin(), type_name.end(), this->name);
				this->name[type_name.size()] = '\0';
				hash = hash::fnv1a_hash(this->name);
			}
		};

		struct ID_lifecycle
		{
			id_t id;
		};

	public:

		template<typename T>
		void add(API_callback_message<T>* msg)
		{
			std::unique_lock lock(lock_);
			callback_message_queue.push(msg);
		}

		void add_create_ID(id_t create_id, const ID_info<> &info = ID_info());

		void add_destroy_ID(id_t destroy_id, const ID_info<> &info = ID_info());

		/// @brief Register Automatic Callback<br>
		/// 호출하는 함수를 API 콜백에 등록합니다.
		/// @tparam T 데이터 타입입니다.
		/// @param obj_id 타켓 객체의 ID입니다.
		/// @param data 추가 데이터입니다.
		/// @param func_info 함수의 정보이며, 현재 함수의 이름을 기본값으로 사용합니다.
		template<typename T>
		void RAC(const id_t obj_id, T* const & data, const func_info_struct<> &func_info = func_info_struct())
		{
			const auto msg = new API_callback_message(obj_id, func_info.hash, data);
			msg->set_func_name(func_info.name);
			add(msg);
		}

		/// @brief Register Automatic Callback<br>
		/// 호출하는 함수를 API 콜백에 등록합니다.
		/// @param obj_id 타켓 객체의 ID입니다.
		/// @param str 추가 데이터입니다.
		/// @param func_info 함수의 정보이며, 현재 함수의 이름을 기본값으로 사용합니다.
		void RAC(const id_t obj_id, const std::string& str, const func_info_struct<> &func_info = func_info_struct())
		{
			struct s
			{
				char* str;
				explicit s(const std::string& str) : str(strdup(str.c_str())) {}
				~s() { free(str); }
			};
			const auto msg = new API_callback_message(obj_id, func_info.hash, new s(str));
			msg->set_func_name(func_info.name);
			add(msg);
		}

		/// @brief Register Automatic Callback<br>
		/// 호출하는 함수를 API 콜백에 등록합니다.
		/// @param obj_id 타켓 객체의 ID입니다.
		/// @param new_id 추가 데이터입니다.
		/// @param func_info 함수의 정보이며, 현재 함수의 이름을 기본값으로 사용합니다.
		void RAC(const id_t obj_id, const id_t new_id, const func_info_struct<> &func_info = func_info_struct())
		{
			struct s
			{
				id_t id;
				explicit s(const id_t id) : id(id) {}
			};
			const auto msg = new API_callback_message(obj_id, func_info.hash, new s(new_id));
			msg->set_func_name(func_info.name);
			add(msg);
		}

		const API_callback_message_base* pop_get();
	};

	extern API_callback_manager callback_manager;
}

#else
namespace uniq::core::api
{
	class API_callback_manager
	{
	public:
		/// @brief UNIQ_DLL_API가 정의되지 않은 경우, 빈 함수로 대체합니다.
		template<typename T>
		void RAC(const id_t obj_id, const T data) {}
	};
}
#endif
