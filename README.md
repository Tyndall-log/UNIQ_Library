<!--
SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
SPDX-License-Identifier: LGPL-3.0-linking-exception
-->

UNIQ Library
============

[![UNIQ](https://custom-icon-badges.demolab.com/badge/-UNIQ-FF8F00.svg)](https://fragrant-alarm-7d3.notion.site/UNIQ-fad2be60e85742268cabce1f06184ac1)
[![JUCE](https://custom-icon-badges.demolab.com/badge/JUCE_v7.0.12-F38D48.svg?logo=JUCE%2032x32)](https://juce.com/)
![C++](https://img.shields.io/badge/-C++20-F34B7D.svg?logo=c%2B%2B&style=flat)
[![license](https://custom-icon-badges.demolab.com/badge/license-LGPL--3.0--linking--exception-green.svg?logo=law)](https://spdx.org/licenses/LGPL-3.0-linking-exception.html)
[![REUSE status](https://api.reuse.software/badge/github.com/Tyndall-log/UNIQ_Library)](https://api.reuse.software/info/github.com/Tyndall-log/UNIQ_Library)

**UNIQ Library**는 유니팩 에디터인 **UNIQ**를 위해 개발된 라이브러리입니다.  
누구나 사용할 수 있도록 라이브러리 형태로 공개합니다.  

## 주요 기능

주요 기능은 다음과 같습니다.
- 런치패드 연결 자동화
	- 모든 런치패드 지원[^각주_런치패드]
- 유니팩 불러오기/저장/편집/미리보기
	- 유니팩 정보 편집
	- 음악 자르기/파형보기/배치
	- LED 편집/배치
	- 자동재생 편집

[^각주_런치패드]: 2024년 8월 이전에 출시된 모든 런치패드 기종

## 문서

내부 함수에 대한 정보는 소스 파일 내 주석(한국어)을 참고해 주세요.  
외부 호출용 함수(API)에 대한 정보와 그 사용 예시는 [UNIQ](https://fragrant-alarm-7d3.notion.site/UNIQ-fad2be60e85742268cabce1f06184ac1)에서 보실 수 있습니다.

## 로드맵

[UNIQ Library 로드맵](https://fragrant-alarm-7d3.notion.site/UNIQ-fad2be60e85742268cabce1f06184ac1)에서 자세히 보실 수 있습니다.

## 종속성

[JUCE](https://www.juce.com/)가 필요합니다.

## 플렛폼 지원

windows, macOS, linux, android, ios

## 빌드

### 요구사항

c++ 컴파일러와 [JUCE](https://www.juce.com/) 프레임워크가 필요합니다.

### 빌드 방법

#### Option 1. CMake (권장)

CMake가 설치되어 있다면, 다음과 같이 빌드할 수 있습니다.

```bash
mkdir build
cd build
cmake ..
cmake --build . --target UNIQ_library
```

#### Option 2. JUCE Projucer

JUCE Projucer가 설치되어 있다면, "UNIQ_library.jucer"를 열어 간단하게 빌드할 수 있습니다.
다만, 종속 라이브러리를 수동으로 설치해야 합니다.

### 지원 컴파일러

- Clang (권장)
- MSVC (Visual Studio)
- GCC

### 최소 빌드 시스템

[JUCE 7](https://juce.com/juce-7-license/)[^각주_JUCE]  
c++20을 지원하는 컴파일러[^각주_컴파일러]

- LLVM 17.0.6 권장
  - Clang >= 16.0.0
  - libc++ >= 17.0.0

[^각주_JUCE]: JUCE v7.0.12 release [github](https://github.com/juce-framework/JUCE/releases/tag/7.0.12)
[^각주_컴파일러]: MSVC v143(Visual Studio 2022) 및 x64-Clang에서 테스트 되었으며, 다른 컴파일러는 추가적인 설정과 인클루드가 필요할 수 있음.

#### Android 빌드 요구사항

Windows 또는 macOS 환경에서 cmake로 구성하는 것을 권장합니다.
아래 cmake 옵션으로 빌드 하세요.  
`-DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21
-DCMAKE_TOOLCHAIN_FILE=<your NDK path>\ndk\<version>\build\cmake\android.toolchain.cmake`  
x86 아키텍처는 `-DANDROID_ABI=armeabi-v7a`로 설정합니다.

- NDK >= r27
	- libc++ >= 18.0.0
	- std::chrono::duration operator<=>가 구현된 최초 버전

## 라이선스

본 리포지토리에서 **명시적**으로 제공되는 모든 파일은 [LGPL-3.0-linking-exception](https://spdx.org/licenses/LGPL-3.0-linking-exception.html)(이하 LGPL3-LE) 라이선스를 따릅니다.
이 라이선스는 LGPL-3.0의 조건을 따르되, 다른 소프트웨어와 링킹할 때 해당 소프트웨어가 LGPL의 일부 제한적인 요건을
따르지 않아도 되는 예외를 허용합니다.

하지만, 이는 본 라이브러리에서 사용되는 모든 소스가 LGPL3-LE와 호환되는 라이선스를 따른다는 것을 의미하지 않습니다.
즉, 본 라이브러리는 다양한 라이브러리를 종속할 수 있으며, 이러한 종속성들은 LGPL3-LE와 호환되지 않는 라이선스를 따를 수 있습니다.
이 경우 본 라이브러리는 더 엄격한 라이선스로 자동 승격됩니다.

### JUCE 종속성에 대한 중요한 안내
본 라이브러리는 CMake 구성을 통해 JUCE 라이브러리의 소스 코드를 자동으로 설치합니다.
이 과정에서 LGPL3-LE 라이선스와 호환되지 않는 JUCE 모듈들이 함께 설치됩니다.
이 경우, 본 라이브러리는 LGPL3-LE 라이선스와 [JUCE 7 EULA](https://juce.com/juce-7-license/) 중 더 엄격한 라이선스를 따릅니다.

---

본 프로젝트는 [SPDX](https://spdx.org/licenses/) 라이선스 규격 및 [REUSE](https://reuse.software/) 프로젝트를 준수합니다.