CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
project(DISASSEMBLEIMAGE)

set(THREADS_PREFER_PTHREAD_FLAG ON)
# 找opencv package----------记得添加这个
find_package(OpenCV REQUIRED) 
find_package(Threads REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

aux_source_directory(. source_list)

add_subdirectory(welcome_regulation)
add_subdirectory(server_info)
add_subdirectory(mask)
add_subdirectory(player_settings_factory)

add_executable(source ${source_list})

link_directories(${OpenCV_LIBRARY_DIRS})

target_link_libraries(source welcomeregulation)
target_link_libraries(source serverinfo)
target_link_libraries(source mask)
target_link_libraries(source playersettingsfactory)

# 链接opencv库
target_link_libraries(source ${OpenCV_LIBS})
target_link_libraries(source Threads::Threads)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
# set(CMAKE_BUILD_TYPE DEBUG)
# set(CMAKE_BUILD_TYPE release)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall")

