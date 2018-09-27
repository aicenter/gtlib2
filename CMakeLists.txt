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

include_directories(${CMAKE_SOURCE_DIR})

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

FIND_PATH(GUROBI_INCLUDE_DIR NAMES gurobi_c++.h PATHS
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



set(SOURCE_FILES base/base.cpp base/base.h base/efg.cpp base/efg.h
        utils/utils.h LPsolvers/LPSolver.h
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
add_executable(main main.cpp ${SOURCE_FILES} ${CPLEX_FILES} ${GLPK_FILES} ${GUROBI_FILES})

target_link_libraries(main ${GUROBILIB} ${CPLEXLIB} ${CMAKE_THREAD_LIBS_INIT} ${GLPKLIB}
        ${Boost_LIBRARIES})



#testing stuff
add_executable(tester tests/matchingPenniesTest.cpp ${SOURCE_FILES} ${CPLEX_FILES} ${GLPK_FILES}
        ${GUROBI_FILES} tests/goofSpielTest.cpp tests/genericPokerTest.cpp)
target_link_libraries(tester ${CPLEXLIB}
        ${CMAKE_THREAD_LIBS_INIT} ${GLPKLIB} ${GUROBILIB} ${Boost_LIBRARIES}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(tester tester)
