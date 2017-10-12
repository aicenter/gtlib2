cmake_minimum_required(VERSION 3.8)
project(pursuit)

set(CMAKE_CXX_STANDARD 11)

# directories with googletest files used for unit tests
add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)
include_directories(lib/googletest/googlemock/include)

# link pthread library
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
endif()

# try to find cplex library with FindCPLEX.cmake code
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")
find_package(CPLEX)

# if found link libraries and add all .h and .cpp files
if( CPLEX_FOUND )
    include_directories(${CPLEX_INCLUDE_DIR})
    link_libraries(${CPLEX_LIBRARY})
    file(GLOB CPLEX_FILES
            "cplexfiles/*.h"
            "cplexfiles/*.cpp"
            )
    set(CPLEXLIB ilocplex)
endif()

set(SOURCE_FILES main.cpp pursuit.h tests/firstTest.cpp base.cpp base.h pursuit.cpp efg.cpp efg.h utils/utils.h)
add_executable(pursuit ${SOURCE_FILES} ${CPLEX_FILES} )
target_link_libraries(pursuit gtest gtest_main ${CPLEXLIB} ${CMAKE_THREAD_LIBS_INIT})
