cmake_minimum_required(VERSION 4.2.1)

project(slTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)


include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
	address_test
	./tests/address_test.cc
	./src/endpoint_info.cc
)

target_link_libraries (
	address_test
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(address_test)
