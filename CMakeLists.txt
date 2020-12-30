cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(network-monitor)

# Dependencies
# If dependencies are managed with conan, we set the CMake module path
# (where Cmake looks for dependencies) to the current build folder.
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif()
find_package(Boost 1.74 REQUIRED COMPONENTS system unit_test_framework)
find_package(OpenSSL 1.1.1)

enable_testing()

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
	OpenSSL::OpenSSL
)

# Export network monitor tests as a executable
set(TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp")
add_executable(network-monitor-tests ${TEST_SOURCES})
target_compile_features(
    network-monitor-tests
    PRIVATE
    cxx_std_17
)
target_compile_definitions(
	network-monitor-tests
	PRIVATE
	TESTS_CACERT_PEM="${CMAKE_CURRENT_SOURCE_DIR}/tests/cacert.pem"
)
target_link_libraries(
    network-monitor-tests
    PRIVATE
    network-monitor-lib
    Boost::Boost
)
add_test(
	NAME
	network-monitor-tests
	COMMAND
	$<TARGET_FILE:network-monitor-tests>
)
set_tests_properties(
	network-monitor-tests
	PROPERTIES
	PASS_REGULAR_EXPRESSION
	".*No errors detected"
)

