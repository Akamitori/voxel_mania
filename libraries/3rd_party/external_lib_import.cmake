# ExternalLibraryImport.cmake

include(ExternalProject)

function(import_external_library LIB_NAME)
    set(options STATIC SHARED)
    set(oneValueArgs
            SOURCE_DIR
            TARGET_NAME
            INSTALL_SUBDIR
            LIB_FILENAME
            DLL_FILENAME
            INCLUDE_SUBDIR
            CMAKE_ARGS
    )
    cmake_parse_arguments(IMPORT_LIB "${options}" "${oneValueArgs}" "" ${ARGN})

    if(NOT IMPORT_LIB_LIB_FILENAME)
        message(FATAL_ERROR "LIB_FILENAME must be set for ${LIB_NAME}")
    endif()

    if (NOT IMPORT_LIB_SOURCE_DIR)
        message(FATAL_ERROR "import_external_library: SOURCE_DIR is required")
    endif ()

    if (NOT IMPORT_LIB_TARGET_NAME)
        set(IMPORT_LIB_TARGET_NAME "${LIB_NAME}::${LIB_NAME}")
    endif ()

    if (NOT IMPORT_LIB_INSTALL_SUBDIR)
        set(IMPORT_LIB_INSTALL_SUBDIR "${LIB_NAME}")
    endif ()

    set(BASE_INSTALL_DIR "${CMAKE_SOURCE_DIR}/.third_party")
    # Ensure INSTALL_PREFIX is defined
    if (NOT DEFINED INSTALL_PREFIX)
        set(INSTALL_PREFIX "${BASE_INSTALL_DIR}/install")
    endif ()

    # Clean up absolute path misuse
    if (IS_ABSOLUTE "${IMPORT_LIB_INSTALL_SUBDIR}")
        message(FATAL_ERROR "INSTALL_SUBDIR should be a relative path, not absolute: '${IMPORT_LIB_INSTALL_SUBDIR}'")
    endif ()

    # Generate the external lib directories
    set(LIB_INSTALL_PATH "${INSTALL_PREFIX}/${IMPORT_LIB_INSTALL_SUBDIR}")
    file(MAKE_DIRECTORY "${LIB_INSTALL_PATH}/lib" "${LIB_INSTALL_PATH}/include" "${LIB_INSTALL_PATH}/bin")

    # Default include subdir
    if (NOT IMPORT_LIB_INCLUDE_SUBDIR)
        set(IMPORT_LIB_INCLUDE_SUBDIR "include")
    endif ()

    # Detect STATIC or SHARED
    if (IMPORT_LIB_STATIC)
        set(LIB_TYPE STATIC)
    elseif (IMPORT_LIB_SHARED)
        set(LIB_TYPE SHARED)
    else ()
        message(FATAL_ERROR "import_external_library: must specify STATIC or SHARED")
    endif ()

    # Set library path
    set(LIB_PATH "${LIB_INSTALL_PATH}/lib/${IMPORT_LIB_LIB_FILENAME}")

    set(SKIP_BUILD OFF)
    file(GLOB_RECURSE HASH_INPUT_FILES
            "${IMPORT_LIB_SOURCE_DIR}/CMakeLists.txt"
            "${IMPORT_LIB_SOURCE_DIR}/*.h"
            "${IMPORT_LIB_SOURCE_DIR}/*.hpp"
            "${IMPORT_LIB_SOURCE_DIR}/*.c"
            "${IMPORT_LIB_SOURCE_DIR}/*.cpp"
    )

    # Combine hash inputs: all source files and CMake args
    list(APPEND HASH_INPUT_FILES ${IMPORT_LIB_CMAKE_ARGS})
    string(SHA256 CURRENT_HASH HASH_COMBINED_STRING)
    # Define path to the hash cache file
    set(HASH_FILE "${LIB_INSTALL_PATH}/.${LIB_NAME}_last_build_hash")

    if(EXISTS "${HASH_FILE}")
        file(READ "${HASH_FILE}" PREVIOUS_HASH)
        string(STRIP "${PREVIOUS_HASH}" PREVIOUS_HASH)
        if("${CURRENT_HASH}" STREQUAL "${PREVIOUS_HASH}")
            message(STATUS "[${LIB_NAME}] Source and config unchanged. Skipping rebuild.")
            set(SKIP_BUILD ON)
        else ()
            message(STATUS "[${LIB_NAME}] Source and config hash mismatch. Initiating rebuild.")
        endif()
    else ()
        message(STATUS "[${LIB_NAME}] is building as an external library")
    endif()
    
    set(EXTERNAL_INSTALL_TARGET "${LIB_NAME}_external-install")
    if (NOT SKIP_BUILD)
        ExternalProject_Add(${LIB_NAME}_external
                SOURCE_DIR "${IMPORT_LIB_SOURCE_DIR}"
                CMAKE_ARGS
                -DCMAKE_INSTALL_PREFIX=${LIB_INSTALL_PATH}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                ${IMPORT_LIB_CMAKE_ARGS}
                INSTALL_DIR "${LIB_INSTALL_PATH}"
                STEP_TARGETS build install
                EXCLUDE_FROM_ALL TRUE
                BUILD_BYPRODUCTS "${LIB_PATH}"
        )

        add_custom_target(${LIB_NAME}_install ALL)
        add_dependencies(${LIB_NAME}_install ${EXTERNAL_INSTALL_TARGET})


        set(TEMP_HASH_FILE "${BASE_INSTALL_DIR}/tmp_${LIB_NAME}_hash.txt")
        add_custom_command(
                TARGET ${EXTERNAL_INSTALL_TARGET}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "${CURRENT_HASH}" > "${TEMP_HASH_FILE}"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${TEMP_HASH_FILE} ${HASH_FILE}
                COMMAND ${CMAKE_COMMAND} -E echo "[${LIB_NAME}] Hash updated at ${HASH_FILE}"
                COMMAND ${CMAKE_COMMAND} -E remove -f
                ${TEMP_HASH_FILE}
                COMMENT "[${LIB_NAME}] Updating hash cache and cleaning up"
        )
    endif ()

    # Declare imported library
    add_library(${IMPORT_LIB_TARGET_NAME} ${LIB_TYPE} IMPORTED GLOBAL)
    if (NOT SKIP_BUILD)
        add_dependencies(${IMPORT_LIB_TARGET_NAME} ${EXTERNAL_INSTALL_TARGET})
    endif ()

    set_target_properties(${IMPORT_LIB_TARGET_NAME} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${LIB_INSTALL_PATH}/${IMPORT_LIB_INCLUDE_SUBDIR}"
    )

    if (LIB_TYPE STREQUAL "STATIC")
        set_target_properties(${IMPORT_LIB_TARGET_NAME} PROPERTIES
                IMPORTED_LOCATION "${LIB_PATH}"
        )
    else ()
        # DLL support: must specify both import lib and runtime DLL
        if (NOT IMPORT_LIB_DLL_FILENAME)
            message(FATAL_ERROR "SHARED libraries must provide DLL_FILENAME for ${LIB_NAME}")
        endif ()

        set(DLL_SOURCE_PATH "${LIB_INSTALL_PATH}/bin/${IMPORT_LIB_DLL_FILENAME}")
        set_target_properties(${IMPORT_LIB_TARGET_NAME} PROPERTIES
                IMPORTED_IMPLIB "${LIB_PATH}"
                IMPORTED_LOCATION "${DLL_SOURCE_PATH}"
        )

        # Use install command to place runtime DLLs into designated binary directory
        install(FILES "${DLL_SOURCE_PATH}"
                DESTINATION "${BINARY_DIRECTORY}"
        )
        
    endif ()
endfunction()
