cmake_minimum_required(VERSION 3.26)

project(UNIQ_library)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*")

set(JUCE_VERSION 7.0.7)
set(JUCE_URL "https://github.com/juce-framework/JUCE/releases/download/")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	set(JUCE_URL "${JUCE_URL}${JUCE_VERSION}/juce-${JUCE_VERSION}-windows.zip")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(JUCE_URL "${JUCE_URL}${JUCE_VERSION}/juce-${JUCE_VERSION}-linux.zip")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	set(JUCE_URL "${JUCE_URL}${JUCE_VERSION}/juce-${JUCE_VERSION}-osx.zip")
endif()

# JUCE 라이브러리를 다운로드 및 빌드합니다.
include(FetchContent)

FetchContent_Declare(
	juce
	URL ${JUCE_URL}
)

FetchContent_MakeAvailable(juce)

add_compile_definitions(JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
include_directories(${juce_SOURCE_DIR}/modules)
include_directories(JuceLibraryCode)

FetchContent_GetProperties(juce)
message(STATUS "juce_SOURCE_DIR: ${juce_SOURCE_DIR}")
message(STATUS "juce_BINARY_DIR: ${juce_BINARY_DIR}")
message(STATUS "juce_POPULATED: ${juce_POPULATED}")
if(NOT juce_POPULATED)
	FetchContent_Populate(juce)
	add_subdirectory(${juce_SOURCE_DIR} ${juce_BINARY_DIR})
endif()

file(GLOB_RECURSE SOURCE_FILES Source/*.cpp)
list(FILTER SOURCE_FILES EXCLUDE REGEX "Source/Console_test/.*")
message(STATUS "SOURCE_FILES: ${SOURCE_FILES}")
foreach(FILE ${SOURCE_FILES})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" RELATIVE_FILE ${FILE})
	message(STATUS "Modified SOURCE_FILE: ${RELATIVE_FILE}")
endforeach()

#add_library(library SHARED library.cpp)
add_executable(UNIQ_library Source/Console_test/main.cpp "${SOURCE_FILES}")
target_link_libraries(UNIQ_library PRIVATE juce::juce_core)
target_link_libraries(UNIQ_library PRIVATE juce::juce_audio_basics)
target_link_libraries(UNIQ_library PRIVATE juce::juce_audio_devices)
target_link_libraries(UNIQ_library PRIVATE juce::juce_audio_formats)
target_link_libraries(UNIQ_library PRIVATE juce::juce_audio_processors)
