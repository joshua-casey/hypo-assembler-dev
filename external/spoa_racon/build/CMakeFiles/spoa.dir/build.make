# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/ritu/hypo_test/hypo/external/spoa

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ritu/hypo_test/hypo/external/spoa/build

# Include any dependencies generated for this target.
include CMakeFiles/spoa.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/spoa.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/spoa.dir/flags.make

CMakeFiles/spoa.dir/src/alignment_engine.cpp.o: CMakeFiles/spoa.dir/flags.make
CMakeFiles/spoa.dir/src/alignment_engine.cpp.o: ../src/alignment_engine.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ritu/hypo_test/hypo/external/spoa/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/spoa.dir/src/alignment_engine.cpp.o"
	/home/ritu/miniconda3/bin/x86_64-conda_cos6-linux-gnu-c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/spoa.dir/src/alignment_engine.cpp.o -c /home/ritu/hypo_test/hypo/external/spoa/src/alignment_engine.cpp

CMakeFiles/spoa.dir/src/alignment_engine.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/spoa.dir/src/alignment_engine.cpp.i"
	/home/ritu/miniconda3/bin/x86_64-conda_cos6-linux-gnu-c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ritu/hypo_test/hypo/external/spoa/src/alignment_engine.cpp > CMakeFiles/spoa.dir/src/alignment_engine.cpp.i

CMakeFiles/spoa.dir/src/alignment_engine.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/spoa.dir/src/alignment_engine.cpp.s"
	/home/ritu/miniconda3/bin/x86_64-conda_cos6-linux-gnu-c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ritu/hypo_test/hypo/external/spoa/src/alignment_engine.cpp -o CMakeFiles/spoa.dir/src/alignment_engine.cpp.s

CMakeFiles/spoa.dir/src/alignment_engine.cpp.o.requires:

.PHONY : CMakeFiles/spoa.dir/src/alignment_engine.cpp.o.requires

CMakeFiles/spoa.dir/src/alignment_engine.cpp.o.provides: CMakeFiles/spoa.dir/src/alignment_engine.cpp.o.requires
	$(MAKE) -f CMakeFiles/spoa.dir/build.make CMakeFiles/spoa.dir/src/alignment_engine.cpp.o.provides.build
.PHONY : CMakeFiles/spoa.dir/src/alignment_engine.cpp.o.provides

CMakeFiles/spoa.dir/src/alignment_engine.cpp.o.provides.build: CMakeFiles/spoa.dir/src/alignment_engine.cpp.o


CMakeFiles/spoa.dir/src/graph.cpp.o: CMakeFiles/spoa.dir/flags.make
CMakeFiles/spoa.dir/src/graph.cpp.o: ../src/graph.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ritu/hypo_test/hypo/external/spoa/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/spoa.dir/src/graph.cpp.o"
	/home/ritu/miniconda3/bin/x86_64-conda_cos6-linux-gnu-c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/spoa.dir/src/graph.cpp.o -c /home/ritu/hypo_test/hypo/external/spoa/src/graph.cpp

CMakeFiles/spoa.dir/src/graph.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/spoa.dir/src/graph.cpp.i"
	/home/ritu/miniconda3/bin/x86_64-conda_cos6-linux-gnu-c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ritu/hypo_test/hypo/external/spoa/src/graph.cpp > CMakeFiles/spoa.dir/src/graph.cpp.i

CMakeFiles/spoa.dir/src/graph.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/spoa.dir/src/graph.cpp.s"
	/home/ritu/miniconda3/bin/x86_64-conda_cos6-linux-gnu-c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ritu/hypo_test/hypo/external/spoa/src/graph.cpp -o CMakeFiles/spoa.dir/src/graph.cpp.s

CMakeFiles/spoa.dir/src/graph.cpp.o.requires:

.PHONY : CMakeFiles/spoa.dir/src/graph.cpp.o.requires

CMakeFiles/spoa.dir/src/graph.cpp.o.provides: CMakeFiles/spoa.dir/src/graph.cpp.o.requires
	$(MAKE) -f CMakeFiles/spoa.dir/build.make CMakeFiles/spoa.dir/src/graph.cpp.o.provides.build
.PHONY : CMakeFiles/spoa.dir/src/graph.cpp.o.provides

CMakeFiles/spoa.dir/src/graph.cpp.o.provides.build: CMakeFiles/spoa.dir/src/graph.cpp.o


CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o: CMakeFiles/spoa.dir/flags.make
CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o: ../src/simd_alignment_engine.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ritu/hypo_test/hypo/external/spoa/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o"
	/home/ritu/miniconda3/bin/x86_64-conda_cos6-linux-gnu-c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o -c /home/ritu/hypo_test/hypo/external/spoa/src/simd_alignment_engine.cpp

CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.i"
	/home/ritu/miniconda3/bin/x86_64-conda_cos6-linux-gnu-c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ritu/hypo_test/hypo/external/spoa/src/simd_alignment_engine.cpp > CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.i

CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.s"
	/home/ritu/miniconda3/bin/x86_64-conda_cos6-linux-gnu-c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ritu/hypo_test/hypo/external/spoa/src/simd_alignment_engine.cpp -o CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.s

CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o.requires:

.PHONY : CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o.requires

CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o.provides: CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o.requires
	$(MAKE) -f CMakeFiles/spoa.dir/build.make CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o.provides.build
.PHONY : CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o.provides

CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o.provides.build: CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o


CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o: CMakeFiles/spoa.dir/flags.make
CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o: ../src/sisd_alignment_engine.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ritu/hypo_test/hypo/external/spoa/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o"
	/home/ritu/miniconda3/bin/x86_64-conda_cos6-linux-gnu-c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o -c /home/ritu/hypo_test/hypo/external/spoa/src/sisd_alignment_engine.cpp

CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.i"
	/home/ritu/miniconda3/bin/x86_64-conda_cos6-linux-gnu-c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ritu/hypo_test/hypo/external/spoa/src/sisd_alignment_engine.cpp > CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.i

CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.s"
	/home/ritu/miniconda3/bin/x86_64-conda_cos6-linux-gnu-c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ritu/hypo_test/hypo/external/spoa/src/sisd_alignment_engine.cpp -o CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.s

CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o.requires:

.PHONY : CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o.requires

CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o.provides: CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o.requires
	$(MAKE) -f CMakeFiles/spoa.dir/build.make CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o.provides.build
.PHONY : CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o.provides

CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o.provides.build: CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o


# Object files for target spoa
spoa_OBJECTS = \
"CMakeFiles/spoa.dir/src/alignment_engine.cpp.o" \
"CMakeFiles/spoa.dir/src/graph.cpp.o" \
"CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o" \
"CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o"

# External object files for target spoa
spoa_EXTERNAL_OBJECTS =

lib/libspoa.a: CMakeFiles/spoa.dir/src/alignment_engine.cpp.o
lib/libspoa.a: CMakeFiles/spoa.dir/src/graph.cpp.o
lib/libspoa.a: CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o
lib/libspoa.a: CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o
lib/libspoa.a: CMakeFiles/spoa.dir/build.make
lib/libspoa.a: CMakeFiles/spoa.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ritu/hypo_test/hypo/external/spoa/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX static library lib/libspoa.a"
	$(CMAKE_COMMAND) -P CMakeFiles/spoa.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/spoa.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/spoa.dir/build: lib/libspoa.a

.PHONY : CMakeFiles/spoa.dir/build

CMakeFiles/spoa.dir/requires: CMakeFiles/spoa.dir/src/alignment_engine.cpp.o.requires
CMakeFiles/spoa.dir/requires: CMakeFiles/spoa.dir/src/graph.cpp.o.requires
CMakeFiles/spoa.dir/requires: CMakeFiles/spoa.dir/src/simd_alignment_engine.cpp.o.requires
CMakeFiles/spoa.dir/requires: CMakeFiles/spoa.dir/src/sisd_alignment_engine.cpp.o.requires

.PHONY : CMakeFiles/spoa.dir/requires

CMakeFiles/spoa.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/spoa.dir/cmake_clean.cmake
.PHONY : CMakeFiles/spoa.dir/clean

CMakeFiles/spoa.dir/depend:
	cd /home/ritu/hypo_test/hypo/external/spoa/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ritu/hypo_test/hypo/external/spoa /home/ritu/hypo_test/hypo/external/spoa /home/ritu/hypo_test/hypo/external/spoa/build /home/ritu/hypo_test/hypo/external/spoa/build /home/ritu/hypo_test/hypo/external/spoa/build/CMakeFiles/spoa.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/spoa.dir/depend

