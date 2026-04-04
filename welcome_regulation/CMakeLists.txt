add_library(welcomeregulation STATIC welcome_regulation.cpp)

target_include_directories(welcomeregulation
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)
