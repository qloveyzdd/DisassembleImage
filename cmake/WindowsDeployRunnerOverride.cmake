if(NOT DEFINED WINDEPLOYQT_EXECUTABLE OR WINDEPLOYQT_EXECUTABLE STREQUAL "")
    message(FATAL_ERROR "windeployqt executable was not found. Set Qt6_DIR or ensure Qt tools are on PATH.")
endif()

if(NOT DEFINED TARGET_FILE OR TARGET_FILE STREQUAL "")
    message(FATAL_ERROR "TARGET_FILE is required for Windows deployment.")
endif()

if(NOT DEFINED DEPLOY_DIR OR DEPLOY_DIR STREQUAL "")
    message(FATAL_ERROR "DEPLOY_DIR is required for Windows deployment.")
endif()

string(REGEX REPLACE "^\"|\"$" "" WINDEPLOYQT_EXECUTABLE "${WINDEPLOYQT_EXECUTABLE}")
string(REGEX REPLACE "^\"|\"$" "" TARGET_FILE "${TARGET_FILE}")
string(REGEX REPLACE "^\"|\"$" "" DEPLOY_DIR "${DEPLOY_DIR}")

file(MAKE_DIRECTORY "${DEPLOY_DIR}")
execute_process(
    COMMAND "${WINDEPLOYQT_EXECUTABLE}" --dir "${DEPLOY_DIR}" "${TARGET_FILE}"
    RESULT_VARIABLE deploy_result
)

if(NOT deploy_result EQUAL 0)
    message(FATAL_ERROR "windeployqt failed with exit code ${deploy_result}.")
endif()

file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES "${TARGET_FILE}"
    RESOLVED_DEPENDENCIES_VAR resolved_dependencies
    UNRESOLVED_DEPENDENCIES_VAR unresolved_dependencies
)

foreach(runtime_dependency IN LISTS resolved_dependencies)
    get_filename_component(runtime_name "${runtime_dependency}" NAME)
    if(runtime_name MATCHES "^api-ms-" OR runtime_name MATCHES "^ext-ms-")
        continue()
    endif()

    file(COPY "${runtime_dependency}" DESTINATION "${DEPLOY_DIR}")
endforeach()

if(unresolved_dependencies)
    list(JOIN unresolved_dependencies ", " unresolved_text)
    message(WARNING "Unresolved runtime dependencies: ${unresolved_text}")
endif()
