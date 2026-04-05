add_library(point_uv STATIC point_uv.cpp)

target_include_directories(point_uv
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(point_uv
    PUBLIC
        ${OpenCV_LIBS}
)
