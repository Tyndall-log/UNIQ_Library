// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "main.h"


namespace ns_test2
{
	using namespace std;
	using namespace juce;
	using namespace uniq;
	
	class test_class_c : public hierarchy
	{
	public:
		chain<int> value_{this, 0, false, &test_class_c::value_};
	};
	class test_class_b : public hierarchy
	{
	public:
		chain<int> value_{this, 0, false, &test_class_b::value_};
	};
	
	class test_class_a : public hierarchy
	{
	public:
		chain<int> value_{this, 0, false, &test_class_a::value_, &test_class_b::value_, &test_class_c::value_};
	};
}

using namespace ns_test2;

int test2()
{
	//메모: decltype: 타입 추론
	
	auto tca_1 = make_shared<test_class_a>();
	auto tca_2 = make_shared<test_class_a>();
	auto tcb_1 = make_shared<test_class_b>();
	auto tcb_2 = make_shared<test_class_b>();
	auto tcb_3 = make_shared<test_class_b>();
	auto tcc_1 = make_shared<test_class_c>();
	
	// 테스트 1: tcb_1을 tca_1의 자식으로 추가하고 동기화 설정 후 값 변경
	tca_1->child_add(tcb_1);
	tcb_1->value_.sync_set(true);
	tcb_1->value_.value_set(100);
	cout << "테스트 1 결과 (tca_1): " << tca_1->value_.value_get() << "\n"; // 예상 출력: 100
	
	// 테스트 2: tca_1에 다른 자식 추가 후 동기화 검증
	tca_1->child_add(tcc_1);
	tcc_1->value_.sync_set(true);
	cout << "테스트 2 결과 (tcc_1): " << tcc_1->value_.value_get() << "\n"; // 예상 출력: 100
	
	// 테스트 3: 동기화 비활성화 후 값 변경 테스트
	tcb_1->value_.sync_set(false);
	tcb_1->value_.value_set(150);
	cout << "테스트 3 결과 (tca_1): " << tca_1->value_.value_get() << "\n"; // 예상 출력: 100
	
	// 테스트 4: 새로운 객체 추가 및 동기화 검증
	tca_1->child_add(tcb_2);
	tcb_2->value_.value_set(200);
	cout << "테스트 4 결과 (tcb_2): " << tcb_2->value_.value_get() << "\n"; // 예상 출력: 200
	cout << "테스트 4 결과 (tca_1): " << tca_1->value_.value_get() << "\n"; // 예상 출력: 100
	
	// 테스트 5: 자식 관계 제거 후 동기화 검증
	tca_1->child_remove(tcb_1);
	tcb_1->value_.value_set(250);
	cout << "테스트 5 결과 (tcb_1): " << tcb_1->value_.value_get() << "\n"; // 예상 출력: 250
	cout << "테스트 5 결과 (tca_1): " << tca_1->value_.value_get() << "\n"; // 예상 출력: 100
	
	return 0;
}