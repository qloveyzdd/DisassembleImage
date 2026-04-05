enable_testing()

add_executable(phase1_smoke
    smoke/Phase1Smoke.cpp
)

add_executable(phase2_smoke
    smoke/Phase2ConfigSmoke.cpp
)

target_link_libraries(phase1_smoke
    PRIVATE
        desktop_runtime
)

target_link_libraries(phase2_smoke
    PRIVATE
        desktop_runtime
)

target_compile_definitions(phase1_smoke
    PRIVATE
        DISASSEMBLEIMAGE_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

target_compile_definitions(phase2_smoke
    PRIVATE
        DISASSEMBLEIMAGE_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

add_test(NAME phase1_smoke COMMAND phase1_smoke)
add_test(NAME phase2_smoke COMMAND phase2_smoke)
set_tests_properties(phase1_smoke PROPERTIES LABELS "smoke")
set_tests_properties(phase2_smoke PROPERTIES LABELS "smoke")
