cmake_minimum_required(VERSION 3.8)
project(pursuit)

set(CMAKE_CXX_STANDARD 11)
add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)
include_directories(lib/googletest/googlemock/include)


set(SOURCE_FILES main.cpp pursuit.h tests/firstTest.cpp base.cpp base.h pursuit.cpp)
add_executable(pursuit ${SOURCE_FILES} tests/firstTest.cpp)
target_link_libraries(pursuit gtest gtest_main)