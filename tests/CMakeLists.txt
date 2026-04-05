enable_testing()

add_executable(phase1_smoke
    smoke/Phase1Smoke.cpp
)

add_executable(phase2_smoke
    smoke/Phase2ConfigSmoke.cpp
)

add_executable(phase3_smoke
    smoke/Phase3Smoke.cpp
)

add_executable(phase4_smoke
    smoke/Phase4Smoke.cpp
)

add_executable(phase5_smoke
    smoke/Phase5Smoke.cpp
)

target_link_libraries(phase1_smoke
    PRIVATE
        desktop_runtime
)

target_link_libraries(phase2_smoke
    PRIVATE
        desktop_runtime
)

target_link_libraries(phase3_smoke
    PRIVATE
        desktop_runtime
)

target_link_libraries(phase4_smoke
    PRIVATE
        desktop_runtime
)

target_link_libraries(phase5_smoke
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

target_compile_definitions(phase3_smoke
    PRIVATE
        DISASSEMBLEIMAGE_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

target_compile_definitions(phase4_smoke
    PRIVATE
        DISASSEMBLEIMAGE_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

target_compile_definitions(phase5_smoke
    PRIVATE
        DISASSEMBLEIMAGE_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

add_test(NAME phase1_smoke COMMAND phase1_smoke)
add_test(NAME phase2_smoke COMMAND phase2_smoke)
add_test(NAME phase3_smoke COMMAND phase3_smoke)
add_test(NAME phase4_smoke COMMAND phase4_smoke)
add_test(NAME phase5_smoke COMMAND phase5_smoke)
set_tests_properties(phase1_smoke PROPERTIES LABELS "smoke")
set_tests_properties(phase2_smoke PROPERTIES LABELS "smoke")
set_tests_properties(phase3_smoke PROPERTIES LABELS "smoke")
set_tests_properties(phase4_smoke PROPERTIES LABELS "smoke")
set_tests_properties(phase5_smoke PROPERTIES LABELS "smoke")
