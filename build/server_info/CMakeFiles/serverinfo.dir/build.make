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
CMAKE_SOURCE_DIR = /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build

# Include any dependencies generated for this target.
include server_info/CMakeFiles/serverinfo.dir/depend.make

# Include the progress variables for this target.
include server_info/CMakeFiles/serverinfo.dir/progress.make

# Include the compile flags for this target's objects.
include server_info/CMakeFiles/serverinfo.dir/flags.make

server_info/CMakeFiles/serverinfo.dir/server_info.cpp.o: server_info/CMakeFiles/serverinfo.dir/flags.make
server_info/CMakeFiles/serverinfo.dir/server_info.cpp.o: ../server_info/server_info.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object server_info/CMakeFiles/serverinfo.dir/server_info.cpp.o"
	cd /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/server_info && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/serverinfo.dir/server_info.cpp.o -c /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/server_info/server_info.cpp

server_info/CMakeFiles/serverinfo.dir/server_info.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/serverinfo.dir/server_info.cpp.i"
	cd /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/server_info && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/server_info/server_info.cpp > CMakeFiles/serverinfo.dir/server_info.cpp.i

server_info/CMakeFiles/serverinfo.dir/server_info.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/serverinfo.dir/server_info.cpp.s"
	cd /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/server_info && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/server_info/server_info.cpp -o CMakeFiles/serverinfo.dir/server_info.cpp.s

# Object files for target serverinfo
serverinfo_OBJECTS = \
"CMakeFiles/serverinfo.dir/server_info.cpp.o"

# External object files for target serverinfo
serverinfo_EXTERNAL_OBJECTS =

server_info/libserverinfo.a: server_info/CMakeFiles/serverinfo.dir/server_info.cpp.o
server_info/libserverinfo.a: server_info/CMakeFiles/serverinfo.dir/build.make
server_info/libserverinfo.a: server_info/CMakeFiles/serverinfo.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libserverinfo.a"
	cd /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/server_info && $(CMAKE_COMMAND) -P CMakeFiles/serverinfo.dir/cmake_clean_target.cmake
	cd /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/server_info && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/serverinfo.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
server_info/CMakeFiles/serverinfo.dir/build: server_info/libserverinfo.a

.PHONY : server_info/CMakeFiles/serverinfo.dir/build

server_info/CMakeFiles/serverinfo.dir/clean:
	cd /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/server_info && $(CMAKE_COMMAND) -P CMakeFiles/serverinfo.dir/cmake_clean.cmake
.PHONY : server_info/CMakeFiles/serverinfo.dir/clean

server_info/CMakeFiles/serverinfo.dir/depend:
	cd /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/server_info /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/server_info /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/server_info/CMakeFiles/serverinfo.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : server_info/CMakeFiles/serverinfo.dir/depend

