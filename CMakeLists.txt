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
find_package(Boost 1.50 REQUIRED COMPONENTS system log_setup log)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Update name and srcs
add_library(server_lib src/server.cc)
add_library(session_lib src/session.cc)

# add handler libs
add_library(echo_handler_lib OBJECT src/echo_request_handler.cc)
add_library(file_handler_lib OBJECT src/file_request_handler.cc)
add_library(not_found_handler_lib OBJECT src/not_found_request_handler.cc)
add_library(handler_lib src/request_handler.cc)
add_library(handler_factory_lib src/request_handler_factory.cc)
add_library(dispatcher_lib src/request_dispatcher.cc)

#set (HANDLER_LIB echo_handler_lib, file_handler_lib, not_found_handler_lib)

#add config parser lib
add_library(config_parser_lib src/config_parser.cc)

#add logger lib
add_library(logger_lib src/logger.cc)


#add and link config parser executable
add_executable(config_parser src/config_parser_main.cc)
target_link_libraries(config_parser config_parser_lib Boost::log)

#add and link config parser test executable
add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test config_parser_lib gtest_main Boost::log)

#add and link echo test executable
add_executable(echo_test tests/echo_handler_test.cc)
target_link_libraries(echo_test $<TARGET_OBJECTS:echo_handler_lib> $<TARGET_OBJECTS:file_handler_lib> $<TARGET_OBJECTS:not_found_handler_lib> handler_factory_lib handler_lib gtest_main Boost::log)

#add and link file handler test executable
add_executable(file_handler_test tests/file_handler_test.cc)
target_link_libraries(file_handler_test handler_factory_lib $<TARGET_OBJECTS:echo_handler_lib> $<TARGET_OBJECTS:file_handler_lib> $<TARGET_OBJECTS:not_found_handler_lib> handler_lib gtest_main Boost::log)

#add and link not found test executable
add_executable(not_found_test tests/not_found_handler_test.cc)
target_link_libraries(not_found_test handler_factory_lib $<TARGET_OBJECTS:not_found_handler_lib> handler_lib gtest_main Boost::log)


#add and link session test executable
add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test session_lib dispatcher_lib handler_factory_lib $<TARGET_OBJECTS:echo_handler_lib> $<TARGET_OBJECTS:file_handler_lib> $<TARGET_OBJECTS:not_found_handler_lib> handler_lib gtest_main Boost::log)

#add and link server test executable
add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test server_lib session_lib config_parser_lib dispatcher_lib handler_factory_lib gtest_main $<TARGET_OBJECTS:echo_handler_lib> $<TARGET_OBJECTS:file_handler_lib> $<TARGET_OBJECTS:not_found_handler_lib> handler_lib  Boost::log)

#add and link logger test executable
add_executable(logger_test tests/logger_test.cc)
target_link_libraries(logger_test logger_lib gtest_main Boost::log_setup Boost::log)


#add and link dispatcher test executable
add_executable(dispatcher_test tests/dispatcher_test.cc)
target_link_libraries(dispatcher_test gtest_main config_parser_lib dispatcher_lib handler_factory_lib $<TARGET_OBJECTS:echo_handler_lib> $<TARGET_OBJECTS:file_handler_lib> $<TARGET_OBJECTS:not_found_handler_lib> handler_lib Boost::log_setup Boost::log)

# add and link integration test executable
string(TOLOWER "${CMAKE_BUILD_TYPE}" LOWER_CMAKE_BUILD_TYPE)
if(NOT "${LOWER_CMAKE_BUILD_TYPE}" STREQUAL "coverage")
    add_executable(integration_test tests/integration_test.cc)
    target_link_libraries(integration_test gtest_main)
endif()

# Update executable name, srcs, and deps
add_executable(webserver src/server_main.cc)
target_link_libraries(webserver server_lib session_lib config_parser_lib dispatcher_lib handler_factory_lib $<TARGET_OBJECTS:echo_handler_lib> $<TARGET_OBJECTS:file_handler_lib> $<TARGET_OBJECTS:not_found_handler_lib>  handler_lib logger_lib Boost::system Boost::log_setup Boost::log)


#gtest
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(echo_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(not_found_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(file_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(logger_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(dispatcher_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)


# ignore integration tests if running for coverage
if(NOT "${LOWER_CMAKE_BUILD_TYPE}" STREQUAL "coverage")
    gtest_discover_tests(integration_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

include(cmake/CodeCoverageReportConfig.cmake)

#coverage
generate_coverage_report(TARGETS webserver config_parser_lib server_lib session_lib handler_factory_lib echo_handler_lib file_handler_lib not_found_handler_lib logger_lib dispatcher_lib TESTS config_parser_test session_test server_test not_found_test echo_test file_handler_test logger_test dispatcher_test) 



