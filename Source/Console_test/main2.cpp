//
// Created by eunsu on 2023-09-24.
//


#include <iostream>
//#include <../JuceLibraryCode/JuceHeader.h>


#include <any>
#include <optional>
#include <typeindex>
#include <memory>
#include <format>

#include "../system.h"

namespace uniq
{
	class Secret
	{
		template<typename T>
		friend class uniq_base;
		class Parent
		{
		public:
			class
			{
				void test() {}
			} parent;
		};
	};

	template<typename T>
	class uniq_base : public ID<T>, public Secret::Parent
	{
	public:
		void test()
		{

		}
	protected:
		uniq_base() = default;
	public:
		class
		{
			void test2() {}
		} test_inlined;
	};

	class chain_configuration
	{

	};

	template<typename T>
	class chain
	{
		T value;
	public:
		const T& get() const
		{
			return value;
		}
		void set(const T& value)
		{
			this->value = value;
		}
		const T& operator=(const T& value)
		{
			set(value);
			return get();
		}
		operator const T& () const { return get(); }
	};

	class A : public ID<A>
	{
	protected:
		A() = default;
	public:
		static std::shared_ptr<A> create()
		{
			return ID<A>::create();
		}
		std::string get_name()
		{
			return "Class A";
		}
	};

	class B : public ID<B>
	{
	protected:
		B() = default;
	public:
		static std::shared_ptr<B> create()
		{
			return ID<B>::create();
		}
		std::string get_name()
		{
			return "Class B";
		}
	};

	class C : public uniq_base<C>
	{
	protected:
		C() = default;
	public:
		static auto create()
		{
			return uniq_base<C>::create();
		}
		std::string get_name()
		{
			return "Class C";
		}
	};
}

int main() {
	std::cout << "Hello, World!" << std::endl;
	juce::String str = "Hello, World!";
	std::cout << str << std::endl;

	system("chcp 65001"); //한글 설정

	using namespace uniq;
	juce::SpinLock lock_;
	juce::SpinLock::ScopedLockType scoped_lock(lock_);

	auto a = A::create();

	return 0;
}