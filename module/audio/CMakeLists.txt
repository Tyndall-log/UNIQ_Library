# SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
# SPDX-License-Identifier: LGPL-3.0-linking-exception

cmake_minimum_required(VERSION 3.26)  #cmake 최소 버전 설정
project(UNIQ_Library_audio)  #프로젝트 이름 설정

set(CMAKE_CXX_STANDARD 20)  #c++20 표준 사용

# 포함할 소스 파일을 찾습니다.
file(GLOB_RECURSE SOURCE_FILES source/*.cpp)  #source 폴더 내의 모든 cpp 파일을 찾습니다.
#message(STATUS "SOURCE_FILES: ${SOURCE_FILES}")
foreach(FILE ${SOURCE_FILES})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" RELATIVE_FILE ${FILE})
	message(STATUS "Modified SOURCE_FILE: ${RELATIVE_FILE}")
endforeach()

if (NOT TARGET UNIQ_Library_core)  #core 모듈이 없다면
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core)
endif()

function(link_library target)  #라이브러리 링크 함수
	target_link_libraries(${target} PRIVATE UNIQ_Library_core)
	target_link_libraries(${target} PRIVATE juce::juce_core)
	target_link_libraries(${target} PRIVATE juce::juce_audio_devices)
	target_link_libraries(${target} PRIVATE juce::juce_audio_formats) #GPL
	if (ANDROID)
		target_link_libraries(${target} PRIVATE oboe)
		target_link_libraries(${target} PRIVATE cpufeatures)
	endif()
endfunction()

file(GLOB CONSOLE_TEST_FILES console_test/*.cpp)  #console_test 폴더 내의 모든 cpp 파일을 찾습니다.
add_executable(UNIQ_Library_audio_test ${CONSOLE_TEST_FILES} ${SOURCE_FILES})  #실행 파일 생성
target_include_directories(UNIQ_Library_audio_test PUBLIC header)  #실행 파일에 header 폴더를 포함합니다.
link_library(UNIQ_Library_audio_test)  #라이브러리 링크

add_library(UNIQ_Library_audio ${SOURCE_FILES})  #라이브러리 생성
target_include_directories(UNIQ_Library_audio PUBLIC header)  #라이브러리에 header 폴더를 포함합니다.
link_library(UNIQ_Library_audio)  #라이브러리 링크