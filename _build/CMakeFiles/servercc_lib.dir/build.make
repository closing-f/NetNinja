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
include CMakeFiles/servercc_lib.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/servercc_lib.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/servercc_lib.dir/flags.make

CMakeFiles/servercc_lib.dir/logger/logger.cpp.o: CMakeFiles/servercc_lib.dir/flags.make
CMakeFiles/servercc_lib.dir/logger/logger.cpp.o: ../logger/logger.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fql/Project/C++/sylar/_build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/servercc_lib.dir/logger/logger.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/servercc_lib.dir/logger/logger.cpp.o -c /home/fql/Project/C++/sylar/logger/logger.cpp

CMakeFiles/servercc_lib.dir/logger/logger.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/servercc_lib.dir/logger/logger.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/fql/Project/C++/sylar/logger/logger.cpp > CMakeFiles/servercc_lib.dir/logger/logger.cpp.i

CMakeFiles/servercc_lib.dir/logger/logger.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/servercc_lib.dir/logger/logger.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/fql/Project/C++/sylar/logger/logger.cpp -o CMakeFiles/servercc_lib.dir/logger/logger.cpp.s

CMakeFiles/servercc_lib.dir/utils.cpp.o: CMakeFiles/servercc_lib.dir/flags.make
CMakeFiles/servercc_lib.dir/utils.cpp.o: ../utils.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fql/Project/C++/sylar/_build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/servercc_lib.dir/utils.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/servercc_lib.dir/utils.cpp.o -c /home/fql/Project/C++/sylar/utils.cpp

CMakeFiles/servercc_lib.dir/utils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/servercc_lib.dir/utils.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/fql/Project/C++/sylar/utils.cpp > CMakeFiles/servercc_lib.dir/utils.cpp.i

CMakeFiles/servercc_lib.dir/utils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/servercc_lib.dir/utils.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/fql/Project/C++/sylar/utils.cpp -o CMakeFiles/servercc_lib.dir/utils.cpp.s

# Object files for target servercc_lib
servercc_lib_OBJECTS = \
"CMakeFiles/servercc_lib.dir/logger/logger.cpp.o" \
"CMakeFiles/servercc_lib.dir/utils.cpp.o"

# External object files for target servercc_lib
servercc_lib_EXTERNAL_OBJECTS =

../lib/libservercc_lib.so: CMakeFiles/servercc_lib.dir/logger/logger.cpp.o
../lib/libservercc_lib.so: CMakeFiles/servercc_lib.dir/utils.cpp.o
../lib/libservercc_lib.so: CMakeFiles/servercc_lib.dir/build.make
../lib/libservercc_lib.so: CMakeFiles/servercc_lib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/fql/Project/C++/sylar/_build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX shared library ../lib/libservercc_lib.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/servercc_lib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/servercc_lib.dir/build: ../lib/libservercc_lib.so

.PHONY : CMakeFiles/servercc_lib.dir/build

CMakeFiles/servercc_lib.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/servercc_lib.dir/cmake_clean.cmake
.PHONY : CMakeFiles/servercc_lib.dir/clean

CMakeFiles/servercc_lib.dir/depend:
	cd /home/fql/Project/C++/sylar/_build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/fql/Project/C++/sylar /home/fql/Project/C++/sylar /home/fql/Project/C++/sylar/_build /home/fql/Project/C++/sylar/_build /home/fql/Project/C++/sylar/_build/CMakeFiles/servercc_lib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/servercc_lib.dir/depend

