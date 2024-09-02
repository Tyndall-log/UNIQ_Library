# SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
# SPDX-License-Identifier: LGPL-3.0-linking-exception

cmake_minimum_required(VERSION 3.26)
project(UNIQ_Library_launchpad)

set(CMAKE_CXX_STANDARD 20)

function(link_library target)
	if (ANDROID)
		target_link_libraries(${target} INTERFACE oboe)
		target_link_libraries(${target} INTERFACE cpufeatures)
	endif()
	target_link_libraries(${target} INTERFACE UNIQ_Library_core_interface)
	target_link_libraries(${target} INTERFACE UNIQ_Library_lightshow_interface)
	target_link_libraries(${target} INTERFACE juce::juce_core)
	target_link_libraries(${target} INTERFACE juce::juce_audio_devices)
endfunction()

# launchpad interface
add_library(UNIQ_Library_launchpad_interface INTERFACE)
file(GLOB_RECURSE SOURCE_FILES source/*.cpp)
target_sources(UNIQ_Library_launchpad_interface INTERFACE ${SOURCE_FILES})
target_include_directories(UNIQ_Library_launchpad_interface INTERFACE header)
link_library(UNIQ_Library_launchpad_interface)

# launchpad static library
add_library(UNIQ_Library_launchpad STATIC)
target_link_libraries(UNIQ_Library_launchpad PRIVATE UNIQ_Library_launchpad_interface)

# launchpad console test
file(GLOB CONSOLE_TEST_FILES console_test/*.cpp)
add_executable(UNIQ_Library_launchpad_test ${CONSOLE_TEST_FILES})
target_link_libraries(UNIQ_Library_launchpad_test PRIVATE UNIQ_Library_launchpad_interface)
