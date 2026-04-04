add_library(playersettingsfactory STATIC player_settings.cpp)

target_include_directories(playersettingsfactory
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(playersettingsfactory
    PUBLIC
        mask
        serverinfo
        Threads::Threads
)
