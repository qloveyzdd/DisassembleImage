CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

aux_source_directory(. mask)

add_library(mask ${mask})

target_link_libraries(mask ${OpenCV_LIBS})