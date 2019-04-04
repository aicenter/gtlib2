cmake_minimum_required(VERSION 3.8)
project(GTLib2)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-m64 -msse2 -O3 -ffast-math")

# link pthread library
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
endif()

# link boost library
include_directories(${Boost_INCLUDE_DIR})
# disable auto link
add_definitions(-DBOOST_ALL_NO_LIB)
# link against dynamic libraries
add_definitions(-DBOOST_TEST_DYN_LINK)
find_package(Boost COMPONENTS unit_test_framework system REQUIRED)

# add cmake
include_directories(${CMAKE_SOURCE_DIR})

# -------------------- link LP solvers --------------------
# ------ CPLEX

# try to find cplex library with FindCPLEX.cmake code
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")
find_package(CPLEX)

# if found link libraries and add all .h and .cpp files
if( CPLEX_FOUND )
    add_definitions(-DCPLEXFOUND=1)
    include_directories(${CPLEX_INCLUDE_DIR})
    link_libraries(${CPLEX_LIBRARY})
    file(GLOB CPLEX_FILES
            "LPsolvers/CplexLPSolver.h"
            "LPsolvers/CplexLPSolver.cpp"
            )
    set(CPLEXLIB ilocplex)
endif()

# ------ GLPK
find_path(GLPK_INCLUDE_DIR NAMES glpk.h PATHS
        ${GLPK_DIR}/basics/glpk/include
        ${GLPK_DIR}/include
        NO_DEFAULT_PATH
        )

find_path(GLPK_LIBRARY  NAMES libglpk.a PATHS
        ${GLPK_DIR}/basics/glpk/lib
        ${GLPK_DIR}/lib
        NO_DEFAULT_PATH
        )

if (GLPK_INCLUDE_DIR AND GLPK_LIBRARY)
    add_definitions(-DGLPKFOUND=1)
    include_directories(${GLPK_INCLUDE_DIR})
    link_libraries(${GLPK_LIBRARY})
    file(GLOB GLPK_FILES
            "LPsolvers/GlpkLPSolver.h"
            "LPsolvers/GlpkLPSolver.cpp"
            )
    set(GLPKLIB glpk)
endif()

# ------ GUROBI
find_path(GUROBI_INCLUDE_DIR NAMES gurobi_c++.h PATHS
        ${GUROBI_DIR}/include
        NO_DEFAULT_PATH
        )

find_PATH(GUROBI_LIBRARY NAMES gurobi80 PATHS
        ${GUROBI_DIR}/lib
        NO_DEFAULT_PATH
        )

if (GUROBI_INCLUDE_DIR AND GUROBI_LIBRARY)
    add_definitions(-DGUROBIFOUND=1)
    include_directories(${GUROBI_INCLUDE_DIR})
    file(GLOB GUROBI_FILES
            "LPsolvers/GurobiLPSolver.h"
            "LPsolvers/GurobiLPSolver.cpp"
            )
    set(GUROBILIB "-L${GUROBI_LIBRARY} -lgurobi_c++ -lgurobi80")
endif()


# -------------------- Source files --------------------

# Individually list all source files.
#
# You could use AUX_SOURCE_DIRECTORY(dir VARIABLE) or globbing files FILE(GLOB MyCSources *.c)
#
# BUT: there is no way for CMake to generate a build system that knows when a new source file
# has been added. Normally the generated build system knows when it needs to rerun CMake because
# the CMakeLists.txt file is modified to add a new source. When the source is just added to the
# directory without modifying this file, one would have to manually rerun CMake to generate
# a build system incorporating the new file.
#
# https://stackoverflow.com/a/2110796
set(SOURCE_FILES
        # algorithms
        algorithms/bestResponse.h algorithms/bestResponse.cpp
        algorithms/cfr.h algorithms/cfr.cpp
        algorithms/common.h algorithms/common.cpp
        algorithms/equilibrium.h algorithms/equilibrium.cpp
        algorithms/normalFormLP.h algorithms/normalFormLP.cpp
        algorithms/stats.h algorithms/stats.cpp
        algorithms/strategy.cpp algorithms/strategy.h
        algorithms/tree.h algorithms/tree.cpp
        algorithms/utility.h algorithms/utility.cpp
        # base
        base/algorithm.cpp base/algorithm.h
        base/base.h base/base.cpp
        base/cache.cpp base/cache.h
        base/efg.h base/efg.cpp
        # domains
        domains/chessboard_factory.h domains/chessboard_factory.cpp
        domains/genericPoker.h domains/genericPoker.cpp
        domains/goofSpiel.h domains/goofSpiel.cpp
        domains/kriegspiel.h domains/kriegspiel.cpp
        domains/matching_pennies.h domains/matching_pennies.cpp
        domains/phantomTTT.h domains/phantomTTT.cpp
        domains/pursuit.h domains/pursuit.cpp
        domains/RhodeIslandPoker.h domains/RhodeIslandPoker.cpp
        domains/oshiZumo.h domains/oshiZumo.cpp
        # LPSolvers
        LPsolvers/LPSolver.h
        # utils
        utils/utils.h
        utils/functools.h
        #utils/highQualityRandom.h utils/highQualityRandom.cpp

)

set(TEST_FILES
        tests/domainsTest.h tests/domainsTest.cpp
        tests/genericPokerTest.cpp
        tests/goofSpielTest.cpp
        tests/kriegSpielTest.cpp
        tests/matchingPenniesTest.cpp
        tests/oshiZumoTest.cpp
        tests/matchingPenniesTest.cpp
        tests/cacheTest.cpp
        tests/cfrTest.cpp
        tests/algorithmTest.cpp

)

# -------------------- Executables, linking --------------------

# main
add_executable(main main.cpp
        ${SOURCE_FILES}
        ${CPLEX_FILES}
        ${GLPK_FILES}
        ${GUROBI_FILES})

target_link_libraries(main
        ${GUROBILIB}
        ${CPLEXLIB}
        ${CMAKE_THREAD_LIBS_INIT}
        ${GLPKLIB}
        ${Boost_LIBRARIES})

# test
add_executable(tester
        ${TEST_FILES}
        ${SOURCE_FILES}
        ${CPLEX_FILES}
        ${GLPK_FILES}
        ${GUROBI_FILES})

target_link_libraries(tester
        ${GUROBILIB}
        ${CPLEXLIB}
        ${CMAKE_THREAD_LIBS_INIT}
        ${GLPKLIB}
        ${Boost_LIBRARIES}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

enable_testing()
add_test(tester tester)
