add_library(LPsolvers OBJECT
        AbstractLPSolver.h
        LPSolver.h
        )
target_include_directories(LPsolvers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(TEST_FILES ${TEST_FILES}
        LPsolvers/LPTest.cpp
        PARENT_SCOPE)


# ----------------------- Try to find LP library -------------------------------

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
        message(STATUS "CPLEX was found ${CPLEX_INCLUDE_DIR} and ${CPLEX_LIBRARY}")
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
        message(STATUS "GLPK was found ${GLPK_INCLUDE_DIR} and ${GLPK_LIBRARY}")
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
        message(STATUS "GUROBI was found ${GUROBI_INCLUDE_DIR} and ${GUROBI_LIBRARY}")
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
    message(STATUS "No LP solver was found, automatically disabling all code related to LPs")
endif()
