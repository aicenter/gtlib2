# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.8

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2017.2\bin\cmake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2017.2\bin\cmake\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\Jacob\CLionProjects\pursuit

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug

# Include any dependencies generated for this target.
include lib/googletest/googlemock/CMakeFiles/gmock_main.dir/depend.make

# Include the progress variables for this target.
include lib/googletest/googlemock/CMakeFiles/gmock_main.dir/progress.make

# Include the compile flags for this target's objects.
include lib/googletest/googlemock/CMakeFiles/gmock_main.dir/flags.make

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/flags.make
lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/includes_CXX.rsp
lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj: ../lib/googletest/googletest/src/gtest-all.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj"
	cd /d C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock && C:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\gmock_main.dir\__\googletest\src\gtest-all.cc.obj -c C:\Users\Jacob\CLionProjects\pursuit\lib\googletest\googletest\src\gtest-all.cc

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.i"
	cd /d C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock && C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Jacob\CLionProjects\pursuit\lib\googletest\googletest\src\gtest-all.cc > CMakeFiles\gmock_main.dir\__\googletest\src\gtest-all.cc.i

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.s"
	cd /d C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock && C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Jacob\CLionProjects\pursuit\lib\googletest\googletest\src\gtest-all.cc -o CMakeFiles\gmock_main.dir\__\googletest\src\gtest-all.cc.s

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj.requires:

.PHONY : lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj.requires

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj.provides: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj.requires
	$(MAKE) -f lib\googletest\googlemock\CMakeFiles\gmock_main.dir\build.make lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj.provides.build
.PHONY : lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj.provides

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj.provides.build: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj


lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/flags.make
lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/includes_CXX.rsp
lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj: ../lib/googletest/googlemock/src/gmock-all.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj"
	cd /d C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock && C:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\gmock_main.dir\src\gmock-all.cc.obj -c C:\Users\Jacob\CLionProjects\pursuit\lib\googletest\googlemock\src\gmock-all.cc

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/gmock_main.dir/src/gmock-all.cc.i"
	cd /d C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock && C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Jacob\CLionProjects\pursuit\lib\googletest\googlemock\src\gmock-all.cc > CMakeFiles\gmock_main.dir\src\gmock-all.cc.i

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/gmock_main.dir/src/gmock-all.cc.s"
	cd /d C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock && C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Jacob\CLionProjects\pursuit\lib\googletest\googlemock\src\gmock-all.cc -o CMakeFiles\gmock_main.dir\src\gmock-all.cc.s

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj.requires:

.PHONY : lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj.requires

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj.provides: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj.requires
	$(MAKE) -f lib\googletest\googlemock\CMakeFiles\gmock_main.dir\build.make lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj.provides.build
.PHONY : lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj.provides

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj.provides.build: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj


lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/flags.make
lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/includes_CXX.rsp
lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj: ../lib/googletest/googlemock/src/gmock_main.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj"
	cd /d C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock && C:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\gmock_main.dir\src\gmock_main.cc.obj -c C:\Users\Jacob\CLionProjects\pursuit\lib\googletest\googlemock\src\gmock_main.cc

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/gmock_main.dir/src/gmock_main.cc.i"
	cd /d C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock && C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Jacob\CLionProjects\pursuit\lib\googletest\googlemock\src\gmock_main.cc > CMakeFiles\gmock_main.dir\src\gmock_main.cc.i

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/gmock_main.dir/src/gmock_main.cc.s"
	cd /d C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock && C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Jacob\CLionProjects\pursuit\lib\googletest\googlemock\src\gmock_main.cc -o CMakeFiles\gmock_main.dir\src\gmock_main.cc.s

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj.requires:

.PHONY : lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj.requires

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj.provides: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj.requires
	$(MAKE) -f lib\googletest\googlemock\CMakeFiles\gmock_main.dir\build.make lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj.provides.build
.PHONY : lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj.provides

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj.provides.build: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj


# Object files for target gmock_main
gmock_main_OBJECTS = \
"CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj" \
"CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj" \
"CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj"

# External object files for target gmock_main
gmock_main_EXTERNAL_OBJECTS =

lib/googletest/googlemock/libgmock_main.a: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj
lib/googletest/googlemock/libgmock_main.a: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj
lib/googletest/googlemock/libgmock_main.a: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj
lib/googletest/googlemock/libgmock_main.a: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/build.make
lib/googletest/googlemock/libgmock_main.a: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX static library libgmock_main.a"
	cd /d C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock && $(CMAKE_COMMAND) -P CMakeFiles\gmock_main.dir\cmake_clean_target.cmake
	cd /d C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\gmock_main.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/googletest/googlemock/CMakeFiles/gmock_main.dir/build: lib/googletest/googlemock/libgmock_main.a

.PHONY : lib/googletest/googlemock/CMakeFiles/gmock_main.dir/build

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/requires: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/__/googletest/src/gtest-all.cc.obj.requires
lib/googletest/googlemock/CMakeFiles/gmock_main.dir/requires: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock-all.cc.obj.requires
lib/googletest/googlemock/CMakeFiles/gmock_main.dir/requires: lib/googletest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj.requires

.PHONY : lib/googletest/googlemock/CMakeFiles/gmock_main.dir/requires

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/clean:
	cd /d C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock && $(CMAKE_COMMAND) -P CMakeFiles\gmock_main.dir\cmake_clean.cmake
.PHONY : lib/googletest/googlemock/CMakeFiles/gmock_main.dir/clean

lib/googletest/googlemock/CMakeFiles/gmock_main.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\Jacob\CLionProjects\pursuit C:\Users\Jacob\CLionProjects\pursuit\lib\googletest\googlemock C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock C:\Users\Jacob\CLionProjects\pursuit\cmake-build-debug\lib\googletest\googlemock\CMakeFiles\gmock_main.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : lib/googletest/googlemock/CMakeFiles/gmock_main.dir/depend
