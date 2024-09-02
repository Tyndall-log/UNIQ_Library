# SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
# SPDX-License-Identifier: LGPL-3.0-linking-exception

cmake_minimum_required(VERSION 3.26)
project(UNIQ_Library_project)

set(CMAKE_CXX_STANDARD 20)

function(link_library target)
	target_link_libraries(${target} INTERFACE juce::juce_core)
	target_link_libraries(${target} INTERFACE UNIQ_Library_core_interface)
	target_link_libraries(${target} INTERFACE UNIQ_Library_audio_interface)
	target_link_libraries(${target} INTERFACE UNIQ_Library_launchpad_interface)
	target_link_libraries(${target} INTERFACE UNIQ_Library_lightshow_interface)
endfunction()

# project interface
add_library(UNIQ_Library_project_interface INTERFACE)
file(GLOB_RECURSE SOURCE_FILES source/*.cpp)
target_sources(UNIQ_Library_project_interface INTERFACE ${SOURCE_FILES})
target_include_directories(UNIQ_Library_project_interface INTERFACE header)
link_library(UNIQ_Library_project_interface)

# project static library
add_library(UNIQ_Library_project STATIC)
target_link_libraries(UNIQ_Library_project PRIVATE UNIQ_Library_project_interface)

# project console test
file(GLOB CONSOLE_TEST_FILES console_test/*.cpp)
add_executable(UNIQ_Library_project_test ${CONSOLE_TEST_FILES})
target_link_libraries(UNIQ_Library_project_test PRIVATE UNIQ_Library_project_interface)
