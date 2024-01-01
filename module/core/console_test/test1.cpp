// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include <utility>

#include "main.h"

using namespace std;
using namespace juce;
using namespace uniq;

class A : public ID<A>
{
protected:
	std::string name;
	A() = default;
	explicit A(std::string name) : name(std::move(name)) {}
public:
//	static std::shared_ptr<A> create()
//	{
//		return ID<A>::create();
//	}
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

class C : public ID<C>
{
protected:
	C() = default;
public:
	static std::shared_ptr<C> create()
	{
		return ID<C>::create();
	}
	std::string get_name()
	{
		return "Class C";
	}
};

int test1()
{
	cout << "Hello World!\n";
	
	MainMessageThread MMT;
	
	auto str = String("Hello juce!");
	cout << str << "\n";
	
	auto a = A::create("name");
	auto b = B::create();
	auto c = C::create();
	//c->test();
	
	//객체 크기 확인
	std::cout << "sizeof(A): " << sizeof(A) << "\n";
	std::cout << "sizeof(B): " << sizeof(B) << "\n";
	std::cout << "sizeof(a): " << sizeof(a) << "\n";
	std::cout << "sizeof(b): " << sizeof(b) << "\n";
	
	auto a_from_ID = ID_manager::get_shared_ptr_by_ID<A>(a->ID_get()).value_or(nullptr);
	if (a_from_ID)
	{
		std::cout << "ID " << a->ID_get() << ": " << a_from_ID->get_name() << "\n";
	}
	
	auto b_from_ID = ID_manager::get_shared_ptr_by_ID<B>(b->ID_get()).value_or(nullptr);
	if (b_from_ID)
	{
		std::cout << "ID " << b->ID_get() << ": " << b_from_ID->get_name() << "\n";
	}
	
	auto test = ID_manager::get_shared_ptr_by_ID<B>(a->ID_get()).value_or(nullptr);
	if (test)
	{
		std::cout << "ID " << a->ID_get() << ": " << test->get_name() << "\n";
	}
	else
	{
		std::cout << "ID " << a->ID_get() << ": " << "nullptr" << "\n";
	}
	
	auto id_2 = 5;
	auto test2 = ID_manager::get_shared_ptr_by_ID<A>(id_2).value_or(nullptr);
	if (test)
	{
		std::cout << "ID " << id_2 << ": " << test->get_name() << "\n";
	}
	else
	{
		std::cout << "ID " << id_2 << ": " << "nullptr" << "\n";
	}
	
	return 0;
}
