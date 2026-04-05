add_library(mask STATIC mask.cpp)

target_include_directories(mask
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(mask
    PUBLIC
        ${OpenCV_LIBS}
        point_uv
)
