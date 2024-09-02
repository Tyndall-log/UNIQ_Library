# SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
# SPDX-License-Identifier: LGPL-3.0-linking-exception

cmake_minimum_required(VERSION 3.26)
project(UNIQ_library)

if(NOT (cxx_std_20 IN_LIST CMAKE_CXX_COMPILE_FEATURES))
	message(FATAL_ERROR "C++20 is not supported by the current compiler.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  #c++20 표준 필수 사용

set(keys "Clang" "AppleClang" "GNU" "MSVC")
set(versions "16.0" "16.0" "11.0" "19.29")
list(FIND keys "${CMAKE_CXX_COMPILER_ID}" index)

if (index EQUAL -1)
	message(WARNING "Build may fail with an unknown compiler. Please check if the compiler is supported.")
	message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
else()
	list(GET versions ${index} required_version)
	list(GET keys ${index} compiler_name)

	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${required_version})
		message(FATAL_ERROR "${compiler_name} version ${required_version} or higher is required. Detected version: ${CMAKE_CXX_COMPILER_VERSION}")
	else()
		message(STATUS "${compiler_name} version ${CMAKE_CXX_COMPILER_VERSION} is being used.")
	endif()

	# TODO:clang libc++ 버전 추가 검사
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	endif ()
endif()

# message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
# message(STATUS "CMAKE_EXPORT_COMPILE_COMMANDS: ${CMAKE_EXPORT_COMPILE_COMMANDS}")
set(UNIQ_INCOMPATIBLE_LICENSE_BAN OFF)  #LGPL-3.0-linking-exception 라이선스와 호환되지 않는 라이선스 사용 금지
option(UNIQ_DLL_API "Enable exporting of UNIQ project DLL API functions" ON)  # UNIQ 프로젝트 DLL API 함수 내보내기 활성화

# ---------- JUCE 라이브러리 다운로드 및 빌드 ----------
if (NOT DEFINED JUCE_VERSION)
	# 필요한 변수 설정
	set(JUCE_VERSION 7.0.12)  # JUCE 버전 설정
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

	if(ANDROID)
		message(STATUS "안드로이드 빌드 설정")
		# 참고: https://atsushieno.github.io/2021/01/16/juce-cmake-android-now-works.html
		if (NOT TARGET oboe)
			set(OBOE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/module/juce/modules/juce_audio_devices/native/oboe")
			if (NOT EXISTS ${OBOE_DIR})
				message(FATAL_ERROR "Oboe 디렉토리가 없습니다.")
			else()
				add_subdirectory(${OBOE_DIR} ./oboe)
			endif()
		endif()
		add_compile_definitions(_LIBCPP_ENABLE_EXPERIMENTAL)

		add_library(cpufeatures STATIC "${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
		#set_source_files_properties("${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
		add_definitions("-DJUCE_ANDROID=1")
		include_directories(AFTER "${ANDROID_NDK}/sources/android/cpufeatures")
		set(CMAKE_SYSTEM_NAME Windows)  #시스템 이름 설정
	endif()

	# JUCE 추가
	add_subdirectory(${JUCE_DIR})
endif()
# ---------- JUCE 라이브러리 다운로드 및 빌드 끝 ----------

add_compile_definitions(JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
if (UNIQ_DLL_API)
	add_compile_definitions(UNIQ_DLL_API=1)
endif()

add_subdirectory(module/core)  #core 모듈 추가
add_subdirectory(module/launchpad)  #launchpad 모듈 추가
add_subdirectory(module/lightshow)  #audio 모듈 추가
add_subdirectory(module/audio)  #audio 모듈 추가
add_subdirectory(module/project)  #uniq 모듈 추가
add_subdirectory(module/unipack)  #unipack 모듈 추가
add_subdirectory(module/workspace)  #workspace 모듈 추가
add_subdirectory(module/api)  #api 모듈 추가

#include_directories(${juce_SOURCE_DIR}/modules)
#include_directories(JuceLibraryCode)

function(link_library target)  #라이브러리 링크 함수
	if (ANDROID)
		target_link_libraries(${target} PRIVATE oboe)
		target_link_libraries(${target} PRIVATE cpufeatures)
	endif()
	target_link_libraries(${target} PRIVATE UNIQ_Library_core_interface)
	target_link_libraries(${target} PRIVATE UNIQ_Library_audio_interface)
	target_link_libraries(${target} PRIVATE UNIQ_Library_lightshow_interface)
	target_link_libraries(${target} PRIVATE UNIQ_Library_launchpad_interface)
	target_link_libraries(${target} PRIVATE UNIQ_Library_project_interface)
	target_link_libraries(${target} PRIVATE UNIQ_Library_unipack_interface)
	target_link_libraries(${target} PRIVATE UNIQ_Library_workspace_interface)
	if (UNIQ_DLL_API)
		target_link_libraries(${target} PRIVATE UNIQ_Library_api_interface)
	endif()
endfunction()

add_library(UNIQ_Library SHARED)  #라이브러리 생성
link_library(UNIQ_Library)  #라이브러리 링크
