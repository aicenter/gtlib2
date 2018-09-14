cmake_minimum_required(VERSION 3.8)
project(GTLib2)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_RELEASE "-msse2 -O3 -ffast-math")

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
            "LPsolvers/simpleCplexSolver.cpp"
            "LPsolvers/simpleCplexSolver.h"
            )
    set(CPLEXLIB ilocplex)
endif()

find_package(Boost COMPONENTS unit_test_framework system REQUIRED)

include_directories(${Boost_INCLUDE_DIR})


FIND_PATH(GLPK_INCLUDE_DIR NAMES glpk.h PATHS
        ${GLPK_DIR}/basics/glpk/include
        ${GLPK_DIR}/include
        NO_DEFAULT_PATH
        )

FIND_PATH(GLPK_LIBRARY  NAMES libglpk.a PATHS
        ${GLPK_DIR}/basics/glpk/lib
        ${GLPK_DIR}/lib
        NO_DEFAULT_PATH
        )

message(${GLPK_INCLUDE_DIR})#set(CMAKE_C_COMPILER "gcc-7")
#set(CMAKE_CXX_COMPILER "g++-7")

# directories with googletest files used for unit tests
#add_subdirectory(lib/googletest)
#include_directories(lib/googletest/googletest/include)
#include_directories(lib/googletest/googlemock/include)
message(${GLPK_LIBRARY})

if (GLPK_INCLUDE_DIR AND GLPK_LIBRARY)


#if (GLPK_FOUND)
    include_directories(${GLPK_INCLUDE_DIR})
    link_libraries(${GLPK_LIBRARY})
    message("bla")
    file(GLOB GLPK_FILES
            "LPsolvers/GlpkLPSolver.h"
            "LPsolvers/GlpkLPSolver.cpp"
            )
    set(GLPKLIB glpk)
endif()


set(SOURCE_FILES base/base.cpp base/base.h base/efg.cpp base/efg.h
        utils/utils.h LPsolvers/LPSolver.cpp LPsolvers/LPSolver.h
        algorithms/normalFormLP.cpp algorithms/normalFormLP.h
        algorithms/treeWalk.cpp algorithms/treeWalk.h
        algorithms/bestResponse.cpp algorithms/bestResponse.h
        algorithms/common.cpp algorithms/common.h
        algorithms/cfr.cpp algorithms/cfr.h
        algorithms/utility.cpp algorithms/utility.h
        algorithms/equilibrium.cpp algorithms/equilibrium.h
        domains/pursuit.h domains/pursuit.cpp domains/phantomTTT.cpp domains/phantomTTT.h
        domains/matching_pennies.cpp domains/matching_pennies.h domains/goofSpiel.cpp domains/goofSpiel.h
        domains/genericPoker.cpp domains/genericPoker.h
        domains/RhodeIslandPoker.cpp domains/RhodeIslandPoker.h
        )
add_executable(main main.cpp ${SOURCE_FILES} ${CPLEX_FILES} ${GLPK_FILES} )

target_link_libraries(main ${CPLEXLIB} ${CMAKE_THREAD_LIBS_INIT} ${GLPKLIB} ${Boost_LIBRARIES})



#testing stuff
add_executable(tester tests/matchingPenniesTest.cpp ${SOURCE_FILES} ${CPLEX_FILES} ${GLPK_FILES} tests/goofSpielTest.cpp tests/genericPokerTest.cpp)
target_link_libraries(tester ${CPLEXLIB}
        ${CMAKE_THREAD_LIBS_INIT} ${GLPKLIB} ${Boost_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(tester tester)
