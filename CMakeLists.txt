cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(network-monitor)

# Dependencies
# If dependencies are managed with conan, we set the CMake module path
# (where Cmake looks for dependencies) to the current build folder.
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif()
find_package(Boost 1.74 REQUIRED COMPONENTS system)

# Export network monitor as a static lib
set(LIB_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/websocket-client.cpp")
add_library(network-monitor-lib STATIC ${LIB_SOURCES})
target_include_directories(
    network-monitor-lib
    PUBLIC
    inc
)
target_compile_features(
    network-monitor-lib
    PUBLIC
    cxx_std_14
)
target_link_libraries(
    network-monitor-lib
    PUBLIC
    Boost::Boost
)

# Export network monitor tests as a executable
set(TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/websocket-client.cpp")
add_executable(network-monitor-tests ${TEST_SOURCES})
target_compile_features(
    network-monitor-tests
    PRIVATE
    cxx_std_14
)
target_link_libraries(
    network-monitor-tests
    PRIVATE
    network-monitor-lib
    Boost::Boost
)


