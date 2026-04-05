add_library(disassemble_core STATIC
    model/FacePositionPrefix.cpp
    model/FacePositionPrefix.h
    model/ProcessingTask.h
    model/ProcessingTypes.h
    model/RunProgress.h
    model/RunResult.h
    io/ImageCatalog.cpp
    io/ImageCatalog.h
    io/LegacyTextConfigLoader.cpp
    io/LegacyTextConfigLoader.h
    engine/CpuDisassemblyRunner.cpp
    engine/CpuDisassemblyRunner.h
)

target_include_directories(disassemble_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}"
)

target_link_libraries(disassemble_core
    PUBLIC
        mask
        point_uv
        serverinfo
        ${OpenCV_LIBS}
        Threads::Threads
)
