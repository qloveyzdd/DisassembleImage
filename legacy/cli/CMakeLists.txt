add_executable(legacy_cli
    main.cpp
)

target_link_libraries(legacy_cli
    PRIVATE
        disassemble_core
        welcomeregulation
)

target_include_directories(legacy_cli
    PRIVATE
        "${CMAKE_SOURCE_DIR}"
)
