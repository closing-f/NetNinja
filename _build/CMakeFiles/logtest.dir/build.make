# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/fql/Project/C++/sylar

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/fql/Project/C++/sylar/_build

# Include any dependencies generated for this target.
include CMakeFiles/logtest.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/logtest.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/logtest.dir/flags.make

CMakeFiles/logtest.dir/test.cpp.o: CMakeFiles/logtest.dir/flags.make
CMakeFiles/logtest.dir/test.cpp.o: ../test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fql/Project/C++/sylar/_build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/logtest.dir/test.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/logtest.dir/test.cpp.o -c /home/fql/Project/C++/sylar/test.cpp

CMakeFiles/logtest.dir/test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/logtest.dir/test.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/fql/Project/C++/sylar/test.cpp > CMakeFiles/logtest.dir/test.cpp.i

CMakeFiles/logtest.dir/test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/logtest.dir/test.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/fql/Project/C++/sylar/test.cpp -o CMakeFiles/logtest.dir/test.cpp.s

# Object files for target logtest
logtest_OBJECTS = \
"CMakeFiles/logtest.dir/test.cpp.o"

# External object files for target logtest
logtest_EXTERNAL_OBJECTS =

../bin/logtest: CMakeFiles/logtest.dir/test.cpp.o
../bin/logtest: CMakeFiles/logtest.dir/build.make
../bin/logtest: ../lib/libservercc_lib.so
../bin/logtest: CMakeFiles/logtest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/fql/Project/C++/sylar/_build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/logtest"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/logtest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/logtest.dir/build: ../bin/logtest

.PHONY : CMakeFiles/logtest.dir/build

CMakeFiles/logtest.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/logtest.dir/cmake_clean.cmake
.PHONY : CMakeFiles/logtest.dir/clean

CMakeFiles/logtest.dir/depend:
	cd /home/fql/Project/C++/sylar/_build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/fql/Project/C++/sylar /home/fql/Project/C++/sylar /home/fql/Project/C++/sylar/_build /home/fql/Project/C++/sylar/_build /home/fql/Project/C++/sylar/_build/CMakeFiles/logtest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/logtest.dir/depend

