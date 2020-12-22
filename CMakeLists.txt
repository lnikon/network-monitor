cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(network-monitor)

# Dependencies
# If dependencies are managed with conan, we set the CMake module path
# (where Cmake looks for dependencies) to the current build folder.
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/conaninfo.txt)
	set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif()

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(network-monitor ${SOURCES})
