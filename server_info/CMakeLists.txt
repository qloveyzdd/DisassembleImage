CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

aux_source_directory(. server_info)

add_library(serverinfo ${server_info})

target_link_libraries(serverinfo ${OpenCV_LIBS})