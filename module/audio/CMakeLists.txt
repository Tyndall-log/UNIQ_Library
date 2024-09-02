# SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
# SPDX-License-Identifier: LGPL-3.0-linking-exception

cmake_minimum_required(VERSION 3.26)
project(UNIQ_Library_audio)

set(CMAKE_CXX_STANDARD 20)

function(link_library target)
	if (ANDROID)
		target_link_libraries(${target} INTERFACE oboe)
		target_link_libraries(${target} INTERFACE cpufeatures)
	endif()
	target_link_libraries(${target} INTERFACE juce::juce_core)
	target_link_libraries(${target} INTERFACE juce::juce_events)
	target_link_libraries(${target} INTERFACE juce::juce_audio_devices)
	target_link_libraries(${target} INTERFACE juce::juce_audio_formats) #GPL
	target_link_libraries(${target} INTERFACE UNIQ_Library_core_interface)
endfunction()

# audio interface
add_library(UNIQ_Library_audio_interface INTERFACE)
file(GLOB_RECURSE SOURCE_FILES source/*.cpp)
target_sources(UNIQ_Library_audio_interface INTERFACE ${SOURCE_FILES})
target_include_directories(UNIQ_Library_audio_interface INTERFACE header)
link_library(UNIQ_Library_audio_interface)

# audio static library
add_library(UNIQ_Library_audio STATIC)
target_link_libraries(UNIQ_Library_audio PRIVATE UNIQ_Library_audio_interface)

# audio console test
file(GLOB CONSOLE_TEST_FILES console_test/*.cpp)
add_executable(UNIQ_Library_audio_test ${CONSOLE_TEST_FILES})
target_link_libraries(UNIQ_Library_audio_test PRIVATE UNIQ_Library_audio_interface)
