CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
project(DISASSEMBLEIMAGE)

# 找opencv package----------记得添加这个
find_package(OpenCV REQUIRED) 

include_directories(${OpenCV_INCLUDE_DIRS})

aux_source_directory(. source_list)

add_subdirectory(server_info)

add_executable(source ${source_list})

link_directories(${OpenCV_LIBRARY_DIRS})

target_link_libraries(source serverinfo)

# 链接opencv库
target_link_libraries(source ${OpenCV_LIBS})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_BUILD_TYPE DEBUG)

