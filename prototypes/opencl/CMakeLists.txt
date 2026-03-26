cmake_minimum_required(VERSION 3.16)
project(OpenCLImageProcessor VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenCL package
find_package(OpenCL REQUIRED)

# Create executable target
add_executable(opencl_test
    main.cpp
)

# Link OpenCL
target_link_libraries(opencl_test PRIVATE OpenCL::OpenCL)

# Set include directories for OpenCL C++ headers
target_include_directories(opencl_test PRIVATE ${OpenCL_INCLUDE_DIRS})

# Copy kernel file to build directory for runtime access
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel.cl
    ${CMAKE_CURRENT_BINARY_DIR}/kernel.cl
    COPYONLY
)

# Compiler-specific options
if(MSVC)
    target_compile_options(opencl_test PRIVATE /W4 /WX-)
else()
    target_compile_options(opencl_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Print configuration info
message(STATUS "OpenCL found: ${OpenCL_FOUND}")
message(STATUS "OpenCL include dirs: ${OpenCL_INCLUDE_DIRS}")
message(STATUS "OpenCL libraries: ${OpenCL_LIBRARIES}")
