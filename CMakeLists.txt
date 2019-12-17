cmake_minimum_required(VERSION 3.8)
project(GTLib)

# --------- Make sure the configuration has been done before compilation -------

# Custom user-specified build configuration goes to file BuildConfiguration.txt
# Default is saved in "example_BuildConfiguration.txt"
set(BUILD_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/BuildConfiguration.txt")

if (NOT EXISTS ${BUILD_CONFIG_FILE})
    message(FATAL_ERROR
            "Local build configuration was not found in '${BUILD_CONFIG_FILE}'."
            " Please follow build instructions in README.md")
else ()
    message(STATUS "Using configuration ${BUILD_CONFIG_FILE}")
endif ()

include(${BUILD_CONFIG_FILE})

# ------------------------- Compilation flags ----------------------------------

# Some explanation of flags is in COMPILATION_NOTES.md

set(CMAKE_CXX_STANDARD 20)
set(COMPILE_FLAGS_WARNINGS "-Wall -Wextra -Wshadow \
                            -Wstrict-aliasing -pedantic -Werror")
# todo: make sure the code passes also other diagnostics
#                            -fmax-errors=5 -Wunreachable-code -Wcast-align \
#                            -Wcast-qual -Wctor-dtor-privacy \
#                            -Wdisabled-optimization -Wformat=2 \
#                            -Winit-self -Wlogical-op -Wmissing-include-dirs \
#                            -Wnoexcept -Wold-style-cast -Woverloaded-virtual \
#                            -Wredundant-decls -Wsign-promo -Wstrict-null-sentinel \
#                            -Wstrict-overflow=5 -Wswitch-default \
#                            -Wno-unused -Wno-variadic-macros -Wno-parentheses \
#                            -fdiagnostics-show-option")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} \
                            -std=c++2a -m64 -O3 \
                            -ffast-math -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} \
                            -std=c++2a -m64 -O0 \
                            -ffast-math -g -fsanitize=address \
                            ${COMPILE_FLAGS_WARNINGS}")


# Make sure it's possible to use includes like "base/efg.h"
include_directories(${CMAKE_SOURCE_DIR})

# ------------------------- Build type (i.e Debug/Release) ---------------------

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' "
            " as none was previously specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING
            "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
else()
    message(STATUS "Using build type '${CMAKE_BUILD_TYPE}'.")
endif ()

ADD_CUSTOM_TARGET(Debug
        COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
        COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
        )

ADD_CUSTOM_TARGET(Release
        COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
        COMMENT "Switch CMAKE_BUILD_TYPE to Release"
        )

# ------------------------------- Documentation --------------------------------

find_package(Doxygen)
if (NOT DOXYGEN_FOUND)
    message(WARNING "doxygen not found - target 'doc' is not available.")
else ()
    message(STATUS "Doxygen was found - target 'doc' is available.")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
            ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif ()

# -------------------------------- Sub-cmakes ----------------------------------

set(GTLIB_OBJECTS
        $<TARGET_OBJECTS:algorithms>
        $<TARGET_OBJECTS:base>
        $<TARGET_OBJECTS:domains>
        $<TARGET_OBJECTS:cli>
        $<TARGET_OBJECTS:LPsolvers>
        $<TARGET_OBJECTS:utils>
        )

add_subdirectory(algorithms)
add_subdirectory(base)
add_subdirectory(domains)
add_subdirectory(cli)
add_subdirectory(LPsolvers)
add_subdirectory(utils)

# ---------------------------------- Tests -------------------------------------

add_subdirectory(external/googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

enable_testing()
add_test(tester tester)

add_executable(tester ${TEST_FILES} ${GTLIB_OBJECTS})
target_link_libraries(tester gtest gtest_main)

# ------------------------------ Executables -----------------------------------

# main
add_executable(main main.cpp ${GTLIB_OBJECTS})



