enable_testing()

add_executable(phase1_smoke
    smoke/Phase1Smoke.cpp
)

target_link_libraries(phase1_smoke
    PRIVATE
        desktop_runtime
)

target_compile_definitions(phase1_smoke
    PRIVATE
        DISASSEMBLEIMAGE_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

add_test(NAME phase1_smoke COMMAND phase1_smoke)
set_tests_properties(phase1_smoke PROPERTIES LABELS "smoke")
