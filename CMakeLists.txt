cmake_minimum_required(VERSION 3.10.0)
project(raddish)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Update name and srcs
add_library(server_lib src/server.cc)
add_library(session_lib src/session.cc)

# add handler libs
add_library(echo_handler_lib src/echo_request_handler.cc)
add_library(file_handler_lib src/file_request_handler.cc)

#add config parser lib
add_library(config_parser_lib src/config_parser.cc)

#add and link config parser executable
add_executable(config_parser src/config_parser_main.cc)
target_link_libraries(config_parser config_parser_lib)

#add and link config parser test executable
add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test config_parser_lib gtest_main)

#add and link echo test executable
add_executable(echo_test tests/echo_handler_test.cc)
target_link_libraries(echo_test echo_handler_lib gtest_main)

#add and link session test executable
add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test session_lib gtest_main echo_handler_lib file_handler_lib)

#add and link server test executable
add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test server_lib session_lib config_parser_lib gtest_main echo_handler_lib file_handler_lib)

# add and link integration test executable
string(TOLOWER "${CMAKE_BUILD_TYPE}" LOWER_CMAKE_BUILD_TYPE)
if(NOT "${LOWER_CMAKE_BUILD_TYPE}" STREQUAL "coverage")
    add_executable(integration_test tests/integration_test.cc)
    target_link_libraries(integration_test gtest_main)
endif()

# Update executable name, srcs, and deps
add_executable(webserver src/server_main.cc)
target_link_libraries(webserver server_lib session_lib config_parser_lib echo_handler_lib file_handler_lib Boost::system)


#gtest
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(echo_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# ignore integration tests if running for coverage
if(NOT "${LOWER_CMAKE_BUILD_TYPE}" STREQUAL "coverage")
    gtest_discover_tests(integration_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

include(cmake/CodeCoverageReportConfig.cmake)

#coverage
generate_coverage_report(TARGETS webserver config_parser_lib server_lib session_lib echo_handler_lib TESTS config_parser_test session_test server_test echo_test) 



