<!--
SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
SPDX-License-Identifier: LGPL-3.0-linking-exception
-->

# Launchpad Module

## 소개

**Launchpad Module**은 **UNIQ_Library**(Launchpad Connector)의 런치패드 모듈입니다.  

**Launchpad Module**은 모든 Novation 런치패드[^각주_런치패드]를 쉽고 편하게 지원하기 위한 고수준 함수를 제공합니다.  
특히 다중 런치패드를 지원하며, 크로스 플랫폼을 지원하도록 개발되었습니다.

> **Launchpad Module**은 **Core Module**을 필요로 합니다.

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

[^각주_런치패드]: 2023년 12월 이전에 출시된 모든 런치패드 기종

## 폴더 구조

폴더 구조는 다음과 같습니다.

- **console_test**  <sub>*모듈 테스트 및 활용 예제*</sub>
- **header**  <sub>*헤더 폴더*</sub>
- **source**  <sub>*소스 폴더*</sub>

## 사용 예시

여기서는 간단한 사용 예시를 소개합니다.  
상세 사용 예시는 `console_test` 폴더 내의 '*.cpp' 파일들을 참고하세요.

> **Launchpad Module**은 **Core Module**을 사용하기 때문에 **Core Module**의 사용 예시를 먼저 보시는 것을 추천합니다.

### 런치패드 연결

```cpp
using namespace std;
using namespace uniq;

int main()
{
	auto MMT = make_unique<MainMessageThread>(); //메시지 관리 스레드 시작
	auto ADM = make_unique<AutoDeviceManager>(); //디바이스 자동 관리 시작
	
	auto input_list = launchpad::get_available_input_list(); //사용 가능한 입력 목록
	if (0 < input_list.size())
	{
		log::println("midi 입력 목록: ");
		for(auto i = 0; i < input_list.size(); ++i)
		{
			auto l = input_list[i];
			log::println("[" + to_string(i) + "]: " + l.name.toStdString());
			log::println("    -> id: " + l.identifier.toStdString());
		}
	}
	else
	{
		log::println("사용 가능한 입력 없음.");
	}
	
	auto output_list = launchpad::get_available_output_list(); //사용 가능한 출력 목록
	if (0 < output_list.size())
	{
		log::println("midi 출력 목록: ");
		for(auto i = 0; i < output_list.size(); ++i)
		{
			auto l = output_list[i];
			log::println("[" + to_string(i) + "]: " + l.name.toStdString());
			log::println("    -> id: " + l.identifier.toStdString());
		}
	}
	else
	{
		log::println("사용 가능한 출력 없음.");
	}
	
	if (input_list.size() <= 0 || output_list.size() <= 0)
	{
		log::println("감지된 런치패드 없음.");
		return 0;
	}
	
	auto lp = launchpad::create(ADM, input_list[0], output_list[0]); //첫 번째 입력, 출력을 사용하여 런치패드 연결 생성
	lp->program_mode_set(true); //프로그래머 모드 활성화
	lp->input_callback_set([](uint8* data, int size) //입력 콜백 설정
	{
		log::println("MIDI_IN: " + String::toHexString(data, size).toStdString()); //입력 메시지 출력
	});
	
	int i = 0;
	cin >> i; //입력 대기
	
	return 0;
}
```