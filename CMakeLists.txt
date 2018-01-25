cmake_minimum_required(VERSION 3.8)
project(GTLib2)

set(CMAKE_CXX_STANDARD 17)

# directories with googletest files used for unit tests
#add_subdirectory(lib/googletest)
#include_directories(lib/googletest/googletest/include)
#include_directories(lib/googletest/googlemock/include)

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
            "LPsolvers/CplexLPSolver.h"
            "LPsolvers/CplexLPSolver.cpp"
            )
    set(CPLEXLIB ilocplex)
endif()

# boost
#set(Boost_INCLUDE_DIR /usr/local/Cellar/boost/1.66.0/include)
#set(Boost_LIBRARY_DIR /usr/local/Cellar/boost/1.66.0/lib)

include_directories(${Boost_INCLUDE_DIR})

find_package(Boost COMPONENTS unit_test_framework REQUIRED)


FIND_PATH(GLPK_INCLUDE_DIR NAMES glpk.h PATHS
        ${GLPK_DIR}/src
        NO_DEFAULT_PATH
        )

FIND_PATH(GLPK_LIB_DIR  NAMES libglpk.so PATHS
        ${GLPK_DIR}/src/.libs
        NO_DEFAULT_PATH
        )

if (GLPK_INCLUDE_DIR AND GLPK_LIB_DIR)
    set(GLPKLIB glpk)
    file(GLOB GLPK_FILES
            "glpkfiles/*.h"
            "glpkfiles/*.cpp"
            )
endif()


set(SOURCE_FILES domains/pursuit.h
        base/base.cpp base/base.h
        domains/pursuit.cpp base/efg.cpp base/efg.h utils/utils.h LPsolvers/LPSolver.cpp LPsolvers/LPSolver.h
        algorithms/normalFormLP.cpp algorithms/normalFormLP.h domains/phantomTTT.cpp domains/phantomTTT.h domains/matching_pennies.cpp domains/matching_pennies.h algorithms/treeWalk.cpp algorithms/treeWalk.h base/efg_old.cpp base/efg_old.h base/base_old.cpp base/base_old.h algorithms/bestResponse.cpp algorithms/bestResponse.h algorithms/common.cpp algorithms/common.h algorithms/utility.cpp algorithms/utility.h algorithms/equilibrium.cpp algorithms/equilibrium.h LPsolvers/simpleCplexSolver.cpp LPsolvers/simpleCplexSolver.h)
add_executable(main  main.cpp ${SOURCE_FILES} ${CPLEX_FILES} ${GLPK_FILES})

target_link_libraries(main
        ${CPLEXLIB}
        ${CMAKE_THREAD_LIBS_INIT} ${GLPKLIB} )



#testing stuff
add_executable(tester tests/matchingPenniesTest.cpp ${SOURCE_FILES} ${CPLEX_FILES} ${GLPK_FILES})
target_link_libraries(tester ${CPLEXLIB}
        ${CMAKE_THREAD_LIBS_INIT} ${GLPKLIB} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(tester tester)
