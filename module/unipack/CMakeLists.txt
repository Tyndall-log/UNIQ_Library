# SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
# SPDX-License-Identifier: LGPL-3.0-linking-exception

cmake_minimum_required(VERSION 3.26)
project(UNIQ_Library_unipack)

set(CMAKE_CXX_STANDARD 20)

function(link_library target)
	target_link_libraries(${target} INTERFACE juce::juce_core)
	target_link_libraries(${target} INTERFACE UNIQ_Library_core_interface)
	target_link_libraries(${target} INTERFACE UNIQ_Library_audio_interface)
	target_link_libraries(${target} INTERFACE UNIQ_Library_launchpad_interface)
	target_link_libraries(${target} INTERFACE UNIQ_Library_lightshow_interface)
	target_link_libraries(${target} INTERFACE UNIQ_Library_project_interface)
endfunction()

# unipack interface
add_library(UNIQ_Library_unipack_interface INTERFACE)
file(GLOB_RECURSE SOURCE_FILES source/*.cpp)
target_sources(UNIQ_Library_unipack_interface INTERFACE ${SOURCE_FILES})
target_include_directories(UNIQ_Library_unipack_interface INTERFACE header)
link_library(UNIQ_Library_unipack_interface)

# unipack static library
add_library(UNIQ_Library_unipack STATIC)
target_link_libraries(UNIQ_Library_unipack PRIVATE UNIQ_Library_unipack_interface)

# unipack console test
file(GLOB CONSOLE_TEST_FILES console_test/*.cpp)
add_executable(UNIQ_Library_unipack_test ${CONSOLE_TEST_FILES})
target_link_libraries(UNIQ_Library_unipack_test PRIVATE UNIQ_Library_unipack_interface)