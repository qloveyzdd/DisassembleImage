if(NOT DEFINED WINDEPLOYQT_EXECUTABLE OR WINDEPLOYQT_EXECUTABLE STREQUAL "")
    message(FATAL_ERROR "windeployqt executable was not found. Set Qt6_DIR or ensure Qt tools are on PATH.")
endif()

if(NOT DEFINED TARGET_FILE OR TARGET_FILE STREQUAL "")
    message(FATAL_ERROR "TARGET_FILE is required for Windows deployment.")
endif()

if(NOT DEFINED DEPLOY_DIR OR DEPLOY_DIR STREQUAL "")
    message(FATAL_ERROR "DEPLOY_DIR is required for Windows deployment.")
endif()

file(MAKE_DIRECTORY "${DEPLOY_DIR}")
execute_process(
    COMMAND "${WINDEPLOYQT_EXECUTABLE}" --dir "${DEPLOY_DIR}" "${TARGET_FILE}"
    RESULT_VARIABLE deploy_result
)

if(NOT deploy_result EQUAL 0)
    message(FATAL_ERROR "windeployqt failed with exit code ${deploy_result}.")
endif()
