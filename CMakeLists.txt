cmake_minimum_required(VERSION 3.8)
project(GTLib2)

# Custom user-specified build configuration goes here
# Default is saved in "example_BuildConfiguration.txt"
set(BUILD_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/BuildConfiguration.txt")

if(NOT EXISTS ${BUILD_CONFIG_FILE})
    message(FATAL_ERROR "Local build configuration was not found in '${BUILD_CONFIG_FILE}'. Please follow build instructions in README.md")
else()
    message("Using configuration ${BUILD_CONFIG_FILE}")
endif()

include(${BUILD_CONFIG_FILE})

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wgnu-designator")
set(CMAKE_CXX_FLAGS_RELEASE "-m64 -msse2 -O3 -ffast-math -DNDEBUG")

# link pthread library
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
if (CMAKE_USE_PTHREADS_INIT)
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
endif ()

# add google test
add_subdirectory(external/googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# add cmake
include_directories(${CMAKE_SOURCE_DIR})

# -------------------- link LP solvers --------------------

# ------ CPLEX
if (USE_CPLEX)
    # try to find cplex library with FindCPLEX.cmake code
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")
    find_package(CPLEX)

    if (NOT CPLEX_FOUND)
        message(FATAL_ERROR "CPLEX was not found, try editing BuildConfiguration.txt")
    endif()

    # if found link libraries and add all .h and .cpp files
    if (CPLEX_FOUND)
        message("CPLEX was found ${CPLEX_INCLUDE_DIR} and ${CPLEX_LIBRARY}")
        add_definitions(-DCPLEXFOUND=1)
        include_directories(${CPLEX_INCLUDE_DIR})
        link_libraries(${CPLEX_LIBRARY})
        file(GLOB CPLEX_FILES
                "LPsolvers/CplexLPSolver.h"
                "LPsolvers/CplexLPSolver.cpp"
                )
        set(CPLEXLIB ilocplex)
    endif ()
endif ()

# ------ GLPK

if (USE_GLPK)
    find_path(GLPK_INCLUDE_DIR NAMES glpk.h PATHS
            ${GLPK_DIR}/basics/glpk/include
            ${GLPK_DIR}/include
            NO_DEFAULT_PATH
            )
    if (NOT GLPK_INCLUDE_DIR)
        message(WARNING "GLPK_INCLUDE_DIR was not found, looking in '${GLPK_DIR}'")
    endif()

    find_path(GLPK_LIBRARY NAMES libglpk.a PATHS
            ${GLPK_DIR}/basics/glpk/lib
            ${GLPK_DIR}/lib
            NO_DEFAULT_PATH
            )
    if (NOT GLPK_LIBRARY)
        message(WARNING "GLPK_LIBRARY was not found, looking in '${GLPK_DIR}'")
    endif()

    if (GLPK_INCLUDE_DIR AND GLPK_LIBRARY)
        message("GLPK was found ${GLPK_INCLUDE_DIR} and ${GLPK_LIBRARY}")
        add_definitions(-DGLPKFOUND=1)
        include_directories(${GLPK_INCLUDE_DIR})
        link_libraries(${GLPK_LIBRARY})
        file(GLOB GLPK_FILES
                "LPsolvers/GlpkLPSolver.h"
                "LPsolvers/GlpkLPSolver.cpp"
                )
        set(GLPKLIB glpk)
    else()
        message(FATAL_ERROR "GLPK was not found, try editing BuildConfiguration.txt")
    endif ()
endif ()

# ------ GUROBI
if (USE_GUROBI)
    find_path(GUROBI_INCLUDE_DIR NAMES gurobi_c++.h PATHS
            ${GUROBI_DIR}/include
            NO_DEFAULT_PATH
            )
    if (NOT GUROBI_INCLUDE_DIR)
        message(WARNING "GUROBI_INCLUDE_DIR was not found, looking in '${GUROBI_DIR}'")
    endif()

    find_path(GUROBI_LIBRARY NAMES libgurobi_c++.a PATHS
            ${GUROBI_DIR}/lib
            NO_DEFAULT_PATH
            )
    if (NOT GUROBI_LIBRARY)
        message(WARNING "GUROBI_LIBRARY was not found, looking in '${GUROBI_DIR}'")
    endif()


    if (GUROBI_INCLUDE_DIR AND GUROBI_LIBRARY)
        message("GUROBI was found ${GUROBI_INCLUDE_DIR} and ${GUROBI_LIBRARY}")
        add_definitions(-DGUROBIFOUND=1)
        include_directories(${GUROBI_INCLUDE_DIR})
        link_libraries(${GUROBI_LIBRARY})
        file(GLOB GUROBI_FILES
                "LPsolvers/GurobiLPSolver.h"
                "LPsolvers/GurobiLPSolver.cpp"
                )
        set(GUROBILIB "-L${GUROBI_LIBRARY} -lgurobi_c++ -lgurobi81")
    else()
        message(FATAL_ERROR "GUROBI was not found, try editing BuildConfiguration.txt")
    endif ()
endif ()

if(NOT USE_CPLEX AND NOT USE_GUROBI AND NOT USE_GLPK)
    message("No LP solver is used")
endif()

# ------------ automatic documentation ----------------

find_package(Doxygen)
if (NOT DOXYGEN_FOUND)
    message(WARNING "doxygen not found - target 'doc' will be unavailable.")
else ()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif ()


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
        algorithms/continualResolving.h
        algorithms/equilibrium.h algorithms/equilibrium.cpp
        algorithms/evaluation.cpp algorithms/evaluation.h
        algorithms/mccr.h algorithms/mccr.cpp
        algorithms/normalFormLP.h algorithms/normalFormLP.cpp
        algorithms/oos.h algorithms/oos.cpp
        algorithms/stats.h algorithms/stats.cpp
        algorithms/strategy.cpp algorithms/strategy.h
        algorithms/utility.h algorithms/utility.cpp
        # base
        base/algorithm.cpp base/algorithm.h
        base/base.h base/base.cpp
        base/cache.cpp base/cache.h
        base/efg.h
        base/fogefg.h base/fogefg.cpp
        base/gadget.h base/gadget.cpp
        base/hashing.h base/hashing.cpp
        base/includes.h
        base/random.h base/random.cpp
        base/tree.h
        # domains
        domains/chessboard_factory.h domains/chessboard_factory.cpp
        domains/genericPoker.h domains/genericPoker.cpp
        domains/goofSpiel.h domains/goofSpiel.cpp
        domains/kriegspiel.h domains/kriegspiel.cpp
        domains/matching_pennies.h domains/matching_pennies.cpp
        domains/normal_form_game.cpp domains/normal_form_game.h
        domains/simple_games.h
        domains/liarsDice.h domains/liarsDice.cpp
        domains/phantomTTT.h domains/phantomTTT.cpp
        domains/pursuit.h domains/pursuit.cpp
        domains/RhodeIslandPoker.h domains/RhodeIslandPoker.cpp
        domains/oshiZumo.h domains/oshiZumo.cpp
        domains/randomGame.h domains/randomGame.cpp
        # LPSolvers
        LPsolvers/AbstractLPSolver.h
        LPsolvers/LPSolver.h
        # utils
        utils/args.hpp
        utils/benchmark.h
        utils/cli_helpers.h
        utils/combinatorics.h utils/combinatorics.cpp
        utils/export.cpp utils/export.h
        utils/export.h utils/export.cpp
        utils/global_args.h
        utils/logging.h
        utils/range.h
        utils/utils.h
        # cereal
        external/cereal/archives/xml.hpp
        external/cereal/archives/adapters.hpp
        external/cereal/archives/portable_binary.hpp
        external/cereal/archives/json.hpp
        external/cereal/archives/binary.hpp
        external/cereal/access.hpp
        external/cereal/details/util.hpp
        external/cereal/details/polymorphic_impl.hpp
        external/cereal/details/traits.hpp
        external/cereal/details/helpers.hpp
        external/cereal/details/polymorphic_impl_fwd.hpp
        external/cereal/details/static_object.hpp
        external/cereal/external/rapidxml/rapidxml_iterators.hpp
        external/cereal/external/rapidxml/rapidxml_print.hpp
        external/cereal/external/rapidxml/rapidxml.hpp
        external/cereal/external/rapidxml/rapidxml_utils.hpp
        external/cereal/external/base64.hpp
        external/cereal/types/stack.hpp
        external/cereal/types/functional.hpp
        external/cereal/types/polymorphic.hpp
        external/cereal/types/base_class.hpp
        external/cereal/types/common.hpp
        external/cereal/types/unordered_set.hpp
        external/cereal/types/utility.hpp
        external/cereal/types/tuple.hpp
        external/cereal/types/concepts/pair_associative_container.hpp
        external/cereal/types/vector.hpp
        external/cereal/types/unordered_map.hpp
        external/cereal/types/chrono.hpp
        external/cereal/types/memory.hpp
        external/cereal/types/map.hpp
        external/cereal/types/array.hpp
        external/cereal/types/valarray.hpp
        external/cereal/types/list.hpp
        external/cereal/types/deque.hpp
        external/cereal/types/string.hpp
        external/cereal/types/queue.hpp
        external/cereal/types/set.hpp
        external/cereal/types/complex.hpp
        external/cereal/types/boost_variant.hpp
        external/cereal/types/bitset.hpp
        external/cereal/types/forward_list.hpp
        external/cereal/cereal.hpp
        external/cereal/macros.hpp
        # experiments
        experiments/benchmark_cfr.h
        experiments/export_domain.h
        experiments/export_ring.h
        experiments/cfr_regrets.h
        )

set(TEST_FILES
        tests/domainsTest.h tests/domainsTest.cpp
        tests/algorithmTest.cpp
        tests/bestResponseTest.cpp
        tests/cacheTest.cpp
        tests/cfrTest.cpp
        tests/crTest.cpp
        tests/commonTest.cpp
        tests/efgTest.cpp
        tests/exportTest.cpp
        tests/gadgetTest.cpp
        tests/genericPokerTest.cpp
        tests/goofSpielTest.cpp
        tests/hashingTest.cpp
        tests/kriegSpielTest.cpp
        tests/LPTest.cpp
        tests/matchingPenniesTest.cpp
        tests/matchingPenniesTest.cpp
        tests/mccrTest.cpp
        tests/oshiZumoTest.cpp
        tests/utilityTest.cpp
        tests/rangeTest.cpp
        tests/oosTest.h tests/oosTest.cpp
        tests/normalFormGameTest.cpp
        tests/randomGameTest.cpp
        tests/liarsDiceTest.cpp
        tests/treeTest.cpp)

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
        ${GLPKLIB})

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
        gtest gtest_main
        )

enable_testing()
add_test(tester tester)
