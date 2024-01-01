# SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
# SPDX-License-Identifier: LGPL-3.0-linking-exception

cmake_minimum_required(VERSION 3.26)  #cmake 최소 버전 설정
project(UNIQ_library)  #프로젝트 이름 설정

set(CMAKE_CXX_STANDARD 20)  #c++20 표준 사용

if(ANDROID)
	# 참고: https://atsushieno.github.io/2021/01/16/juce-cmake-android-now-works.html
	set(OBOE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/module/juce/modules/juce_audio_devices/native/oboe")
	add_subdirectory(${OBOE_DIR} ./oboe)

	add_library(cpufeatures STATIC "${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
	#set_source_files_properties("${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
	add_definitions("-DJUCE_ANDROID=1")
	include_directories(AFTER "${ANDROID_NDK}/sources/android/cpufeatures")
	set(CMAKE_SYSTEM_NAME Windows)  #시스템 이름 설정
endif()

# ---------- JUCE 라이브러리 다운로드 및 빌드 ----------
if (NOT DEFINED JUCE_VERSION)
	# 필요한 변수 설정
	set(JUCE_VERSION 7.0.9)  # JUCE 버전 설정
	set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/module/juce")  # JUCE 디렉토리 설정
	set(JUCE_VERSION_FILE "${JUCE_DIR}/.version")  # JUCE 버전 정보를 위한 파일 경로 설정
	set(JUCE_LOCK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/module")  # 파일 잠금을 위한 디렉토리 경로 설정

	# 파일 잠금 로직
	file(LOCK ${JUCE_LOCK_DIR} DIRECTORY GUARD FILE TIMEOUT 120)

	# JUCE 디렉토리와 버전 체크
	set(NEEDS_DOWNLOAD OFF)
	if (EXISTS ${JUCE_DIR})
		if (EXISTS ${JUCE_VERSION_FILE})
			file(READ ${JUCE_VERSION_FILE} JUCE_STATUS_CONTENT)
			if (NOT ${JUCE_STATUS_CONTENT} STREQUAL ${JUCE_VERSION})
				message(STATUS "JUCE 버전이 다릅니다. (현재 버전: ${JUCE_STATUS_CONTENT}, 요구 버전: ${JUCE_VERSION})")
				set(NEEDS_DOWNLOAD ON)
			endif()
		else()
			message(STATUS "JUCE 버전 정보 파일이 없습니다.")
			set(NEEDS_DOWNLOAD ON)
		endif()
	else()
		message(STATUS "JUCE 모듈이 없습니다.")
		set(NEEDS_DOWNLOAD ON)
	endif()

	if(NEEDS_DOWNLOAD)
		# JUCE 다운로드 로직
		include(FetchContent)
		FetchContent_Declare(
			juce
			GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
			GIT_TAG ${JUCE_VERSION}
			SOURCE_DIR ${JUCE_DIR}
		)
		message(STATUS "JUCE 다운로드 시작")
		FetchContent_Populate(juce)
		message(STATUS "JUCE 다운로드 완료")

		# 버전 정보 업데이트
		file(WRITE ${JUCE_VERSION_FILE} "${JUCE_VERSION}")
	endif()

	# 파일 잠금 해제 및 잔여 파일 제거
	file(LOCK ${JUCE_LOCK_DIR} DIRECTORY RELEASE)
	file(REMOVE ${JUCE_LOCK_DIR}/cmake.lock RESULT_VARIABLE JUCE_REMOVE_RESULT)
	if (NOT ${JUCE_REMOVE_RESULT} EQUAL 0)
		message(STATUS "잔여 파일 제거 실패")
	endif()

	# JUCE 추가
	add_subdirectory(${JUCE_DIR})
endif()
# ---------- JUCE 라이브러리 다운로드 및 빌드 끝 ----------

add_subdirectory(module/core)  #core 모듈 추가
add_subdirectory(module/launchpad)  #launchpad 모듈 추가
add_subdirectory(module/audio)  #audio 모듈 추가

add_compile_definitions(JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
#include_directories(${juce_SOURCE_DIR}/modules)
#include_directories(JuceLibraryCode)

#function(link_library target)  #라이브러리 링크 함수
#	target_link_libraries(${target} PRIVATE UNIQ_Library_core)
#	target_link_libraries(${target} PRIVATE UNIQ_Library_launchpad)
#	target_link_libraries(${target} PRIVATE juce::juce_core)
#	target_link_libraries(${target} PRIVATE juce::juce_audio_devices)
#	if (ANDROID)
#		target_link_libraries(${target} PRIVATE oboe)
#		target_link_libraries(${target} PRIVATE cpufeatures)
#	endif()
#endfunction()
