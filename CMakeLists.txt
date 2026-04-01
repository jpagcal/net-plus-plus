cmake_minimum_required(VERSION 4.2.1)

project(slTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(
	SOURCE_FILES
	./src/endpoint_info.cc
	./src/error.cc
	./src/io_context.cc
	./src/tcp.cc
	./src/resolver.cc
)
set(
	TEST_FILES
	./tests/address_test.cc
	./tests/address_info_test.cc
)

find_package(PkgConfig REQUIRED)
if (PKG_CONFIG_FOUND)
	pkg_check_modules(LIBEVENT REQUIRED IMPORTED_TARGET libevent)
	if (LIBEVENT_FOUND)
		message(STATUS "found")
	endif()
endif()

include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

include(GoogleTest)


foreach(testfile ${TEST_FILES})
	string(REGEX MATCH "\./tests/(.+)\.cc" _ ${testfile})

	add_executable(
		${CMAKE_MATCH_1}
		${testfile}
		${SOURCE_FILES}
	)
	target_link_libraries(
		${CMAKE_MATCH_1}
		GTest::gtest_main
		PkgConfig::LIBEVENT
	)

	gtest_discover_tests(${CMAKE_MATCH_1})
endforeach()
