<!--
SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
SPDX-License-Identifier: LGPL-3.0-linking-exception
-->

# Launchpad Module

## 소개

**Launchpad Module**은 **UNIQ_Library**의 런치패드 모듈입니다.  

**Launchpad Module**은 모든 Novation 런치패드[^각주_런치패드]를 쉽고 편하게 지원하기 위한 고수준 함수를 제공합니다.  
특히 다중 런치패드를 지원하며, 크로스 플랫폼을 지원하도록 개발되었습니다.

> **Launchpad Module**은 **Core Module**과 **Lightshow Module**을 필요로 합니다.

## 주요 기능

주요 기능은 다음과 같습니다.

- 모든 Novation 런치패드 지원[^각주_런치패드]
- 연결된 런치패드 목록(기종 확인)
- 런치패드 연결 자동화
- 런치패드 입력 감지
- 런치패드 버튼(LED) 색 설정 ({고정 빈도, 즉시 전송} 모드 지원)
- 프로그래머 모드 자동 진입
- 다중 런치패드 동시 지원
- 런치패드 전용 간편한 고수준 함수

[^각주_런치패드]: 2024년 9월 이전에 출시된 모든 런치패드 기종

## 폴더 구조

폴더 구조는 다음과 같습니다.

- **console_test**  <sub>*모듈 테스트 및 활용 예제*</sub>
- **header**  <sub>*헤더 폴더*</sub>
- **source**  <sub>*소스 폴더*</sub>

## 사용 예시

여기서는 간단한 사용 예시를 소개합니다.  
상세 사용 예시는 `console_test` 폴더 내의 '*.cpp' 파일들을 참고하세요.

> **Launchpad Module**은 **Core Module**과 **Lightshow Module**을 사용하기 때문에 두 모듈의 사용 예시를 먼저 보시는 것을 권장합니다.

### 런치패드 연결

```cpp
#include <random>
#include "launchpad.h"

using namespace std;
using namespace uniq;

int main()
{
	auto adm = audio_device_manager::get();
	auto lm = launchpad::launchpad_manager::instance_get();
	auto lpl = lm->launchpad_list_get();
	cout << "연결할 런치패드 목록:" << endl;
	for (auto i = 0; i < lpl.size(); i++)
	{
		cout << "\t[" + to_string(i) + "] : " + lpl[i]->input_kind_name_get() << endl;
	}
	if (lpl.empty())
	{
		cout << "연결할 런치패드가 없습니다." << endl;
		return -1;
	}
	const auto lp = lpl[0]; // 첫 번째 런치패드 선택
	cout << "선택된 런치패드: " << lp->input_kind_name_get() << endl;
	lp->program_mode_set();

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution dis(0, 255);
	const auto ibdc = lp->input_button_down_callback_add([&](const uint8_t x, const uint8_t y, const uint8_t velocity) {
		cout << "버튼 눌림: " + to_string(x) + ", " + to_string(y) + ", " + to_string(velocity) << endl;
		// lp->velocity_set(x, y, velocity);
		lp->rgb_set(x, y, dis(gen), dis(gen), dis(gen));
		});
	const auto ibuc = lp->input_button_up_callback_add([&](const uint8_t x, const uint8_t y) {
		cout << "버튼 떼어짐: " + to_string(x) + ", " + to_string(y) << endl;
		// lp->velocity_set(x, y, 0);
		lp->rgb_set(x, y, 0, 0, 0);
	});

	cout << "종료하려면 엔터를 누르세요." << endl;
	cin.get();
	lp->input_button_down_callback_remove(ibdc);
	lp->input_button_up_callback_remove(ibuc);
	lp->program_mode_set(false);
	
	return 0;
}
```