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
include CMakeFiles/source.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/source.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/source.dir/flags.make

CMakeFiles/source.dir/main.cpp.o: CMakeFiles/source.dir/flags.make
CMakeFiles/source.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/source.dir/main.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/source.dir/main.cpp.o -c /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/main.cpp

CMakeFiles/source.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/source.dir/main.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/main.cpp > CMakeFiles/source.dir/main.cpp.i

CMakeFiles/source.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/source.dir/main.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/main.cpp -o CMakeFiles/source.dir/main.cpp.s

# Object files for target source
source_OBJECTS = \
"CMakeFiles/source.dir/main.cpp.o"

# External object files for target source
source_EXTERNAL_OBJECTS =

source: CMakeFiles/source.dir/main.cpp.o
source: CMakeFiles/source.dir/build.make
source: welcome_regulation/libwelcomeregulation.a
source: server_info/libserverinfo.a
source: mask/libmask.a
source: player_settings_factory/libplayersettingsfactory.a
source: point_uv/libpoint_uv.a
source: server_info/libserverinfo.a
source: mask/libmask.a
source: /usr/local/lib/libopencv_dnn.so.3.4.16
source: /usr/local/lib/libopencv_highgui.so.3.4.16
source: /usr/local/lib/libopencv_ml.so.3.4.16
source: /usr/local/lib/libopencv_objdetect.so.3.4.16
source: /usr/local/lib/libopencv_shape.so.3.4.16
source: /usr/local/lib/libopencv_stitching.so.3.4.16
source: /usr/local/lib/libopencv_superres.so.3.4.16
source: /usr/local/lib/libopencv_videostab.so.3.4.16
source: /usr/local/lib/libopencv_calib3d.so.3.4.16
source: /usr/local/lib/libopencv_features2d.so.3.4.16
source: /usr/local/lib/libopencv_flann.so.3.4.16
source: /usr/local/lib/libopencv_photo.so.3.4.16
source: /usr/local/lib/libopencv_video.so.3.4.16
source: /usr/local/lib/libopencv_videoio.so.3.4.16
source: /usr/local/lib/libopencv_imgcodecs.so.3.4.16
source: /usr/local/lib/libopencv_imgproc.so.3.4.16
source: /usr/local/lib/libopencv_core.so.3.4.16
source: CMakeFiles/source.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable source"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/source.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/source.dir/build: source

.PHONY : CMakeFiles/source.dir/build

CMakeFiles/source.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/source.dir/cmake_clean.cmake
.PHONY : CMakeFiles/source.dir/clean

CMakeFiles/source.dir/depend:
	cd /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build /mnt/3f800fcd-15e8-42fa-b544-5713adb2f9bf/DisassembleImage.git/build/CMakeFiles/source.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/source.dir/depend

