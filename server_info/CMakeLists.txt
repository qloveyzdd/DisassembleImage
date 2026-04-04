add_library(serverinfo STATIC server_info.cpp)

target_include_directories(serverinfo
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(serverinfo
    PUBLIC
        ${OpenCV_LIBS}
)
