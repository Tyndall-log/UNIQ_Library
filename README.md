<!--
SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
SPDX-License-Identifier: LGPL-3.0-linking-exception
-->

UNIQ Library
============

[![UNIQ](https://custom-icon-badges.demolab.com/badge/-UNIQ-FF8F00.svg)](https://fragrant-alarm-7d3.notion.site/UNIQ-fad2be60e85742268cabce1f06184ac1)
[![JUCE](https://custom-icon-badges.demolab.com/badge/JUCE_v7.0.5-F38D48.svg?logo=JUCE%2032x32)](https://juce.com/)
![C++](https://img.shields.io/badge/-C++20-F34B7D.svg?logo=c%2B%2B&style=flat)
[![license](https://custom-icon-badges.demolab.com/badge/license-LGPL--3.0--linking--exception-green.svg?logo=law)](https://spdx.org/licenses/LGPL-3.0-linking-exception.html)
[![REUSE status](https://api.reuse.software/badge/github.com/IreneStella/UNIQ_Library)](https://api.reuse.software/info/github.com/IreneStella/UNIQ_Library)

**UNIQ Library**는 유니팩 에디터인 **UNIQ**를 위해 개발된 라이브러리입니다.  
누구나 사용할 수 있도록 라이브러리 형태로 공개합니다.  

## 주요 기능
주요 기능은 다음과 같습니다.
- 런치패드 연결 자동화
    - 모든 런치패드 지원 (2023 기준)
- 유니팩 불러오기/저장/편집/미리보기 (구현중)
    - 유니팩 정보 편집
    - 음악 자르기/파형보기/배치
    - LED 편집/배치
    - 자동재생 편집

## 문서

내부 함수에 대한 정보는 소스 파일 내 주석(한국어)을 참고해 주세요.  
외부 호출용 함수(API)에 대한 정보와 그 사용 예시는 [UNIQ](https://fragrant-alarm-7d3.notion.site/UNIQ-fad2be60e85742268cabce1f06184ac1)에서 보실 수 있습니다.

## 로드맵

[UNIQ Library 로드맵](https://fragrant-alarm-7d3.notion.site/UNIQ-fad2be60e85742268cabce1f06184ac1)에서 자세히 보실 수 있습니다.

## 종속성

[JUCE](https://www.juce.com/)가 필요합니다.

## 빌드

c++ 컴파일러와 [JUCE](https://www.juce.com/) 프레임워크가 필요합니다.  
"UNIQ_library.jucer"를 열어 빌드 설정 후 빌드를 진행하시면 됩니다.

### 최소 빌드 시스템

[JUCE 7](https://juce.com/juce-7-license/)  
c++20을 지원하는 컴파일러

### 대상

windows

## 라이선스

본 리포지토리의 모든 소스는 [LGPL-3.0-linking-exception](https://spdx.org/licenses/LGPL-3.0-linking-exception.html) 라이선스를 따릅니다.
