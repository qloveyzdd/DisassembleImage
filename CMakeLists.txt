cmake_minimum_required(VERSION 3.26)

project(DisassembleImage LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(DISASSEMBLEIMAGE_BUILD_DESKTOP "Build the Qt desktop shell" ON)
option(DISASSEMBLEIMAGE_BUILD_TESTS "Build smoke tests" ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CTest)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

if(DISASSEMBLEIMAGE_BUILD_DESKTOP)
    find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(WindowsDeploy)
include(WindowsDeployOverride)

add_subdirectory(welcome_regulation)
add_subdirectory(point_uv)
add_subdirectory(mask)
add_subdirectory(server_info)
add_subdirectory(core)
add_subdirectory(app/windows)
add_subdirectory(legacy/cli)

if(DISASSEMBLEIMAGE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

include(CPack)
