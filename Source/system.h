// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "JuceHeader.h"
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

	class ID_manager
	{
		template<typename T> friend class ID;
		static std::unordered_map<std::size_t, std::any> registry_;
		static std::size_t id_; //0은 무효한 ID입니다.
		static juce::SpinLock lock;
	private:
		static std::size_t generate_ID();
		template<typename T>
		static void register_ID(std::size_t id, std::shared_ptr<T> obj)
		{
			juce::SpinLock::ScopedLockType scoped_lock(lock);
			registry_[id] = std::weak_ptr<T>(obj);
		}
		static void unregister_ID(std::size_t id);
	public:
		template<typename T>
		static std::optional<std::shared_ptr<T>> get_shared_ptr_by_ID(std::size_t id)
		{
			juce::SpinLock::ScopedLockType scoped_lock(lock);
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
				throw std::runtime_error("ID_manager::get_shared_ptr_by_ID: ID " + std::to_string(id) + " is not registered."
					+ std::format("Please check if the object is created by {}::create().", typeid(T).name()));
			}
			return std::nullopt;
		}
	};

	//public ID<T> 상속을 통해 ID_manager에 ID를 생성하고 등록하는 클래스를 만듭니다.
	//부여 받은 ID는 ID_manager를 통해 해당 객체를 참조할 수 있습니다.
	//해당 클래스는 shared_ptr를 위해 ID<T>::create()를 통해 객체를 생성하도록 강제하므로,
	//객체가 임의로 생성되지 않도록 생성자를 private로 선언하는 것을 권장합니다.
	template<typename T>
	class ID
	{
	private:
		std::size_t id_ = 0; //0은 무효한 ID입니다.
	protected:
		ID() : id_(ID_manager::generate_ID()) {};
		~ID()
		{
			ID_manager::unregister_ID(id_);
		}
	public:
		ID(const ID&) = delete;
		ID& operator=(const ID&) = delete;
		ID(ID&&) = delete;
		ID& operator=(ID&&) = delete;
		template <typename... K>
		static std::shared_ptr<T> create(K &&...args)
		{
			//make_shared에 프라이빗 생성자를 사용하기 위한 구조체
			//컴파일 최적화로 MakeSharedEnabler의 오버 헤드는 없음.
			struct MakeSharedEnabler : public T
			{
				MakeSharedEnabler(K &&...args) : T(std::forward<K>(args)...) {}
			};
			std::shared_ptr<T> sp = std::make_shared<MakeSharedEnabler>(std::forward<K>(args)...);

			ID_manager::register_ID(sp->id_, sp);
			return sp;
		}
		size_t get_ID() const
		{
			return id_;
		}
	};
	

	//콘솔에서 메인 스레드와 독립적으로 메시지 이벤트 처리할 수 있도록 하는 클래스
	class MainMessageThread : public juce::Thread, public juce::Component
	{
	public:
		MainMessageThread();
		~MainMessageThread();

		void run() override;
	};

	//사용자 정의 리터럴을 구현합니다.
	inline consteval unsigned char operator "" _uc(unsigned long long arg) noexcept { return static_cast<unsigned char>(arg); }

	template<size_t N>
	struct hex_convert
	{
		juce::uint8 data[N] = {};

		inline consteval hex_convert(const char(&s)[N], const char delim = '\'') noexcept
		{
			auto hex_char_to_uint8 = [](const char c) {
				if (c >= '0' && c <= '9')
					return static_cast<juce::uint8>(c - '0');
				else if (c >= 'A' && c <= 'F')
					return static_cast<juce::uint8>(c - 'A' + 10);
				else if (c >= 'a' && c <= 'f')
					return static_cast<juce::uint8>(c - 'a' + 10);
				else
					return static_cast<juce::uint8>(0);  // Unexpected character
			};

			juce::uint8 tmp = 0;
			bool reading_first_digit = true;
			size_t data_idx = 0;
			for (size_t i = 0; i < N - 1; ++i)
			{
				if (s[i] == delim) continue;

				juce::uint8 val = hex_char_to_uint8(s[i]);
				if (reading_first_digit)
				{
					tmp = val << 4;
				}
				else
				{
					tmp |= val;
					data[data_idx++] = tmp;
				}

				reading_first_digit = !reading_first_digit;
			}
		}
	};

	template<hex_convert HC>
	inline consteval auto operator "" _hex() noexcept
	{
		return HC.data;
	}
}