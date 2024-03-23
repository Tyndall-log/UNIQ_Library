# SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
# SPDX-License-Identifier: LGPL-3.0-linking-exception

cmake_minimum_required(VERSION 3.26)  #cmake 최소 버전 설정
project(UNIQ_Library_core)  #프로젝트 이름 설정

set(CMAKE_CXX_STANDARD 20)  #c++20 표준 사용

## JUCE 라이브러리를 다운로드 및 빌드합니다.
#set(JUCE_VERSION 7.0.8)  #juce 버전 설정
#set(JUCE_URL "https://github.com/juce-framework/JUCE/releases/download/")  #juce 다운 경로
#if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
#	set(JUCE_URL "${JUCE_URL}${JUCE_VERSION}/juce-${JUCE_VERSION}-windows.zip")
#elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
#	set(JUCE_URL "${JUCE_URL}${JUCE_VERSION}/juce-${JUCE_VERSION}-linux.zip")
#elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
#	set(JUCE_URL "${JUCE_URL}${JUCE_VERSION}/juce-${JUCE_VERSION}-osx.zip")
#else()
#	message(FATAL_ERROR "Unsupported system: ${CMAKE_SYSTEM_NAME}")
#endif()
#include(FetchContent)  #다운로드 도구 활성화
#FetchContent_Declare(
#		juce
#		URL ${JUCE_URL}
#		SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../juce  # 다운로드될 디렉토리 지정
#)  #juce 다운로드 설정
#FetchContent_MakeAvailable(juce)  #juce 설치

# 포함할 소스 파일을 찾습니다.
file(GLOB_RECURSE SOURCE_FILES source/*.cpp)  #source 폴더 내의 모든 cpp 파일을 찾습니다.
#message(STATUS "SOURCE_FILES: ${SOURCE_FILES}")
foreach(FILE ${SOURCE_FILES})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" RELATIVE_FILE ${FILE})
	message(STATUS "Modified SOURCE_FILE: ${RELATIVE_FILE}")
endforeach()

function(link_library target)  #라이브러리 링크 함수
	target_link_libraries(${target} PRIVATE juce::juce_core)
	target_link_libraries(${target} PRIVATE juce::juce_events)
endfunction()

file(GLOB CONSOLE_TEST_FILES console_test/*.cpp)  #console_test 폴더 내의 모든 cpp 파일을 찾습니다.
add_executable(UNIQ_Library_core_test ${CONSOLE_TEST_FILES} ${SOURCE_FILES} header)  #실행 파일을 만듭니다.
target_include_directories(UNIQ_Library_core_test PUBLIC header)  #실행 파일에 header 폴더를 포함합니다.
link_library(UNIQ_Library_core_test)  #실행 파일에 라이브러리를 링크합니다.

add_library(UNIQ_Library_core ${SOURCE_FILES})  #core 라이브러리를 만듭니다.
target_include_directories(UNIQ_Library_core PUBLIC header)  #core 라이브러리에 header 폴더를 포함합니다.
link_library(UNIQ_Library_core)  #core 라이브러리에 라이브러리를 링크합니다.