set(_windows_deploy_module_dir "${CMAKE_CURRENT_LIST_DIR}")

function(configure_windows_deploy target_name)
    if(NOT WIN32)
        return()
    endif()

    get_target_property(_target_type "${target_name}" TYPE)
    if(NOT _target_type STREQUAL "EXECUTABLE")
        return()
    endif()

    find_program(WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt.debug.bat windeployqt windeployqt.exe
        HINTS
            "${Qt6_DIR}/../../../bin"
            "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/tools/Qt6/bin"
            "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/tools/Qt6/bin"
    )
    if(NOT WINDEPLOYQT_EXECUTABLE)
        message(WARNING "windeployqt was not found. windows_deploy_check will fail until Qt tools are available.")
    endif()

    set(_deploy_dir "${CMAKE_BINARY_DIR}/deploy/${target_name}")

    add_custom_command(
        TARGET "${target_name}" POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${_deploy_dir}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${target_name}>" "${_deploy_dir}/"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/input.obj" "${_deploy_dir}/"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/output.obj" "${_deploy_dir}/"
        VERBATIM
    )

    add_custom_target(windows_deploy_check
        COMMAND "${CMAKE_COMMAND}"
            -DDEPLOY_DIR="${_deploy_dir}"
            -DWINDEPLOYQT_EXECUTABLE="${WINDEPLOYQT_EXECUTABLE}"
            -DTARGET_FILE="$<TARGET_FILE:${target_name}>"
            -P "${_windows_deploy_module_dir}/WindowsDeployRunnerOverride.cmake"
        DEPENDS "${target_name}"
        COMMENT "Collecting Windows runtime dependencies for ${target_name}"
        VERBATIM
    )
endfunction()
