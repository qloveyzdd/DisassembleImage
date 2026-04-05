if(NOT DISASSEMBLEIMAGE_BUILD_DESKTOP)
    return()
endif()

qt_standard_project_setup()

add_library(desktop_runtime STATIC
    EnvironmentCheck.cpp
    EnvironmentCheck.h
    MainWindow.cpp
    MainWindow.h
    RunController.cpp
    RunController.h
    TaskRunSession.cpp
    TaskRunSession.h
    TaskFormState.cpp
    TaskFormState.h
    TaskFormValidator.cpp
    TaskFormValidator.h
    TaskPresetStore.cpp
    TaskPresetStore.h
)

target_link_libraries(desktop_runtime
    PUBLIC
        disassemble_core
        Qt${QT_VERSION_MAJOR}::Widgets
        ${OpenCV_LIBS}
        Threads::Threads
)

target_include_directories(desktop_runtime
    PUBLIC
        "${CMAKE_SOURCE_DIR}"
)

qt_add_executable(disassemble_desktop
    AppMain.cpp
)

target_link_libraries(disassemble_desktop
    PRIVATE
        desktop_runtime
)

set_target_properties(disassemble_desktop PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "DisassembleImageDesktop"
)

configure_windows_deploy(disassemble_desktop)
