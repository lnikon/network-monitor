cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(network-monitor)

# Dependencies
# If dependencies are managed with conan, we set the CMake module path
# (where Cmake looks for dependencies) to the current build folder.
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif()
find_package(Boost 1.74 REQUIRED COMPONENTS system unit_test_framework)
find_package(OpenSSL 1.1.1 REQUIRED)
find_package(CURL 7.74 REQUIRED)
find_package(nlohmann_json 3.9.1 REQUIRED)
find_package(spdlog 1.8.5 REQUIRED)

enable_testing()

# Export network monitor as a static lib
set(LIB_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/websocket-client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/file-downloader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/transport-network.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/stomp-frame.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/stomp-parser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/stomp-client.cpp"
)

add_library(network-monitor-lib STATIC ${LIB_SOURCES})
target_include_directories(
    network-monitor-lib
    PUBLIC
    inc
)
target_compile_features(
    network-monitor-lib
    PUBLIC
    cxx_std_17
)
target_link_libraries(
    network-monitor-lib
    PUBLIC
    Boost::Boost
    OpenSSL::OpenSSL
    CURL::CURL
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

# Export network monitor tests as a executable
set(
    TEST_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/file-downloader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/transport-network.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/client-websocket.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/stomp-client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/mock-websocket-client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/stomp-parser-test.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/boost-spirit-test.cpp"
)
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
    TESTS_NETWORK_LAYOUT_JSON="${CMAKE_CURRENT_SOURCE_DIR}/tests/network-layout.json"
)
target_include_directories(
    network-monitor-tests
    PUBLIC
    inc
)
target_link_libraries(
    network-monitor-tests
    PRIVATE
    network-monitor-lib
    Boost::Boost
    OpenSSL::OpenSSL
    CURL::CURL
    nlohmann_json::nlohmann_json
    spdlog::spdlog
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

# Export networking playground
set(
    NETWORK_PLAYGROUND_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/network-playground.cpp"
)
add_executable(network-playground ${NETWORK_PLAYGROUND_SOURCES})
target_compile_features(
    network-playground
    PRIVATE
    cxx_std_17
)
target_compile_definitions(
    network-playground
    PRIVATE
    TESTS_CACERT_PEM="${CMAKE_CURRENT_SOURCE_DIR}/tests/cacert.pem"
    TESTS_NETWORK_LAYOUT_JSON="${CMAKE_CURRENT_SOURCE_DIR}/tests/network-layout.json"
)
target_include_directories(
    network-playground
    PUBLIC
    inc
)
target_link_libraries(
    network-playground
    PRIVATE
    network-monitor-lib
    Boost::Boost
    OpenSSL::OpenSSL
    CURL::CURL
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)
