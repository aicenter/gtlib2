cmake_minimum_required(VERSION 3.8)
project(GTLib2)

# --------- Make sure the configuration has been done before compilation -------

# Custom user-specified build configuration goes to file BuildConfiguration.txt
# Default is saved in "example_BuildConfiguration.txt"
set(BUILD_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/BuildConfiguration.txt")

if(NOT EXISTS ${BUILD_CONFIG_FILE})
    message(FATAL_ERROR
            "Local build configuration was not found in '${BUILD_CONFIG_FILE}'. "
            "Please follow build instructions in README.md")
else()
    message("Using configuration ${BUILD_CONFIG_FILE}")
endif()

include(${BUILD_CONFIG_FILE})

# ------------------------- Compilation flags ----------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wgnu-designator -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-m64 -msse2 -O3 -ffast-math -DNDEBUG")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ------------------------- Build type (i.e Debug/Release) ---------------------
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

# add google test
add_subdirectory(external/googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# add cmake
include_directories(${CMAKE_SOURCE_DIR})

# ------------------------------- Documentation --------------------------------

find_package(Doxygen)
if (NOT DOXYGEN_FOUND)
    message(WARNING "doxygen not found - target 'doc' is not available.")
else ()
    message("-- Doxygen was found - target 'doc' is available.")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif ()

# -------------------------------- Sub-cmakes ----------------------------------

add_subdirectory(algorithms)
add_subdirectory(base)
add_subdirectory(domains)
add_subdirectory(cli)
add_subdirectory(LPsolvers)
add_subdirectory(utils)

set (GTLIB_OBJECTS
        $<TARGET_OBJECTS:algorithms>
        $<TARGET_OBJECTS:base>
        $<TARGET_OBJECTS:domains>
        $<TARGET_OBJECTS:cli>
        $<TARGET_OBJECTS:LPsolvers>
        $<TARGET_OBJECTS:utils>
        )

# -------------------- Source files --------------------

set(TEST_FILES
        tests/ISMCTSTest.cpp
        tests/strategoTest.cpp
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
        tests/gambitTest.cpp
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
add_executable(main main.cpp ${GTLIB_OBJECTS})

# test
enable_testing()
add_executable(tester ${TEST_FILES} ${GTLIB_OBJECTS})
target_link_libraries(tester gtest gtest_main)
add_test(tester tester)
