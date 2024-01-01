<!--
SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
SPDX-License-Identifier: LGPL-3.0-linking-exception
-->

# Core Module

## 소개

**Core Module**은 **UNIQ_Library**(Launchpad Connector)의 핵심 모듈입니다.

**Core Module**은 객체 소유권 관리, 객체 ID 번호 할당 및 ID 번호로 객체 특정, 상하위 객체의 연결 관리,
상하위 객체의 상속 여부에 따른 변수 값 자동 동기화, 메시지 스레드 생성 등의 기능을 제공합니다.

## 주요 기능

주요 기능은 다음과 같습니다.

- 객체 소유권 관리 자동화
- 객체 ID 번호 할당 및 ID 번호로 객체 특정
- 상하위 객체의 연결 관리 자동화
- 상하위 객체의 상속 여부에 따른 변수 값 자동 동기화
- 메시지 스레드 생성
- 편리한 log 기능

## 폴더 구조

폴더 구조는 다음과 같습니다.

- **console_test**  <sub>*모듈 테스트 및 활용 예제*</sub>
- **header**  <sub>*헤더 폴더*</sub>
- **source**  <sub>*소스 폴더*</sub>

## 사용 예시

여기서는 간단한 사용 예시를 소개합니다.  
상세 사용 예시는 `console_test` 폴더 내의 '*.cpp' 파일들을 참고하세요.  

### 객체 소유권 관리

객체 소유권 관리는 `ID` 클래스를 상속받아 구현합니다.  
`ID` 클래스를 상속받은 클래스는 `create` 함수를 통해 객체를 생성해야 합니다.  
`create` 함수는 객체의 `std::shared_ptr`를 반환합니다.  
잘못된 생성자 호출을 방지하기 위해 생성자는 protected나 private으로 선언해야 합니다.  

아래는 `A` 클래스가 `ID` 클래스를 상속받아 구현한 예시입니다.  

```cpp
using namespace std;
using namespace uniq;

class A : public ID<A> // ID 클래스를 상속받아 구현합니다. 이때, 템플릿 인자로 자기 자신을 넣습니다.
{
public:
	string name;
protected: // 생성자는 protected나 private으로 선언해야 합니다.
	A() = default; // 기본 생성자를 만듭니다.
	A(string name) : name(name) {} // 원하는 인자를 받는 생성자를 만듭니다.
};

int main()
{
	std::shared_ptr<A> a;
	{
		auto a1 = A::create("a1"); // 객체 생성
		auto a2 = A::create("a2"); // 객체 생성
		a = a2; // 소유권 공유
	} // 범위를 벗어나면서 a1의 객체는 소멸하지만, a2의 객체는 a와 공유하므로 소멸하지 않습니다.
	
	cout << a->name << endl; // a2  
	return 0;
}
```

### 객체 ID 번호 할당 및 ID 번호로 객체 특정

객체 ID 번호 할당 및 ID 번호로 객체 특정도 `ID` 클래스를 상속받아 구현합니다.  

```cpp
using namespace std;
using namespace uniq;

class A : public ID<A>
{
public:
	string name;
protected:
	A(string name) : name(name) {}
};

class B : public ID<B>
{
public:
	string name;
protected:
	B(string name) : name(name) {}
};

int main()
{
	// 객체 생성
	auto a1 = A::create("a1");
	auto b1 = B::create("b1");
	
	// 객체 ID 번호 얻기
	auto a1_id = a1->ID_get();
	auto b1_id = b1->ID_get();
	
	cout << a1_id << endl; // 1
	cout << b1_id << endl; // 2
	
	// ID 번호로 객체 특정
	auto a1_from_ID = ID_manager::get_shared_ptr_by_ID<A>(a1_id).value_or(nullptr);
	if (a1_from_ID != nullptr)
		cout << a1_from_ID->name << endl; // a1
	else
		cout << "a1 is nullptr" << endl; // 도달 불가능
	
	auto b1_from_ID = ID_manager::get_shared_ptr_by_ID<B>(b1_id).value_or(nullptr);
	if (b1_from_ID != nullptr)
		cout << b1_from_ID->name << endl; // b1
	else
		cout << "b1 is nullptr" << endl; // 도달 불가능
	
	// ID 번호로 객체 특정 (실패)
	auto a1_from_ID_fail = ID_manager::get_shared_ptr_by_ID<A>(b1_id).value_or(nullptr); //b1_id는 A의 ID가 아니므로 실패
	if (a1_from_ID_fail != nullptr)
		cout << a1_from_ID_fail->name << endl; // 도달 불가능
	else
		cout << "a1_from_ID_fail is nullptr" << endl; // a1_from_ID_fail is nullptr
	
	return 0;
}
```

### 상하위 객체의 연결 관리

상하위 객체의 연결 관리는 `hierarchy` 클래스를 상속받아 구현합니다.  
상하위 객체는 `child_add` 함수와 `parent_add` 함수를 통해 연결합니다.  
`child_add` 함수는 하위 객체를 추가하고, `parent_add` 함수는 상위 객체를 추가합니다.  
상위 객체와 하위 객체는 여러개를 가질 수 있지만, 순환 참조는 허용하지 않습니다.  
상하위 객체의 해제는 `child_remove` 함수와 `parent_remove` 함수를 통해 해제합니다.  

```cpp
using namespace std;
using namespace uniq;

class A : public hierarchy // hierarchy 클래스를 상속받아 구현합니다.
{};

class B : public hierarchy
{};

int main()
{
	// 객체 생성
	auto a1 = make_shared<A>();
	auto a2 = make_shared<A>();
	auto b1 = make_shared<B>();
	auto b2 = make_shared<B>();
	
	// 상하위 객체 연결
	a1->child_add(a2); // a1의 하위 객체로 a2를 추가합니다.
	b1->parent_add(a1); // b1의 상위 객체로 a1을 추가합니다. (=a1->child_add(b1);)
	b1->child_add(b2); // b1의 하위 객체로 b2를 추가합니다.
	
	// 상하위 객체 연결 해제
	a1->parent_remove(a2); // a1의 상위 객체 a2를 해제합니다. (=a2->child_remove(a1);)
	a1->child_remove(b1); // a1의 하위 객체 b1을 해제합니다.
	b2->parent_remove(b1); // b2의 상위 객체 b1을 해제합니다. (=b1->child_remove(b2);)
	
	return 0;
}
```

### 상하위 객체의 상속 여부에 따른 변수 값 자동 동기화

상하위 객체의 상속 여부에 따른 변수 값 자동 동기화도 `hierarchy` 클래스를 상속받아 구현합니다.  
상하위 객체와 동기화 할 멤버 변수는 `chain<T>` 타입으로 선언합니다.  
이때, `T`는 동기화 할 변수의 타입입니다.  
`chain<T>` 생성자의 인자는 아래와 같습니다.  

`chain<T>(this, value, sync, member_ptr...)`
- `this` : 자신의 포인터 (항상 `this`를 넣어야 합니다.)
- `value` : 변수의 초기값 (타입 `T`)
- `sync` : 부모 객체의 변수와 동기화 여부 (타입 `bool`)
- `member_ptr...` : 자동으로 동기화 할 클래스의 멤버 포인터 (가변 인자)

설명이 복잡하지만, 아래 예시를 보면 이해가 쉽습니다.  

```cpp
using namespace std;
using namespace uniq;

class A : public ID<A>, public hierarchy // ID 클래스와 함께 쓰면 더 편리합니다.
{
public:
	chain<int> a{this, 0, true, &A::a}; // a와 동기화합니다.
};

class B : public ID<B>, public hierarchy
{
public:
	chain<int> a{this, 0, true, &B::a, &A::a}; // a 및 A의 a와 동기화합니다.
};

int main()
{
	// 객체 생성
	auto a1 = A::create();
	auto b1 = B::create();
	
	// 상하위 객체 연결
	b1->parent_add(a1);
	
	// 동기화 확인
	cout << a1->a << endl; // 0
	cout << b1->a << endl; // 0
	
	// 동기화
	a1->a = 1;
	
	// 동기화 확인
	cout << a1->a << endl; // 1
	cout << b1->a << endl; // 1
	
	// 동기화 해제
	b1->a.sync_set(false); // b1의 부모 객체와 동기화를 해제합니다.
	// b1->parent_remove(a1); //도 가능합니다.
	a1->a = 2;
	
	// 동기화 확인
	cout << a1->a << endl; // 2
	cout << b1->a << endl; // 1
	
	b1->a.sync_set(true); // b1의 부모 객체와 동기화를 다시 설정합니다.
	// b1->parent_add(a1); //도 가능합니다.
	
	// 동기화 확인
	cout << a1->a << endl; // 2
	cout << b1->a << endl; // 2
	
	return 0;
}
```

이 밖에도 `chain<T>`는 다양한 멤버 함수를 제공합니다.  
더 복잡한 예시는 `console_test` 폴더 내의 ~~`hierarchy_test.cpp`~~(임시 `test2.cpp`) 파일을 참고해 주세요.

### 메시지 스레드 생성

메시지 스레드 생성은 `MainMessageThread` 클래스를 생성하면 됩니다.

```cpp
using namespace std;
using namespace uniq;

int main()
{
	{
		auto MMT = make_unique<MainMessageThread>(); //생성과 동시에 메시지 관리 스레드 시작됩니다.
	} // 범위를 벗어나면서 메시지 관리 스레드가 종료됩니다.
	return 0;
}
```

### 편리한 log 기능

편리한 log 기능은 `log` 클래스를 사용하면 됩니다.

```cpp
using namespace std;
using namespace uniq;

int main()
{
	log::println("Hello World!"); // Hello World!
	log::print("hi, uniq!\n"); // hi, uniq!
	return 0;
}
```