CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

aux_source_directory(. player_settings_factory)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../server_info)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../mask)

add_library(playersettingsfactory ${player_settings_factory})

target_link_libraries(playersettingsfactory PRIVATE serverinfo)
target_link_libraries(playersettingsfactory PRIVATE mask)