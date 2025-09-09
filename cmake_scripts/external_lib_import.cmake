include(ExternalProject)

function(import_external_library LIB_NAME)
    set(options STATIC SHARED)
    set(oneValueArgs SOURCE_DIR TARGET_NAME)
    set(multiValueArgs CMAKE_ARGS)
    cmake_parse_arguments(IMPORT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Validate required args
    if(NOT IMPORT_SOURCE_DIR)
        message(FATAL_ERROR "${LIB_NAME}: SOURCE_DIR is required")
    endif()

    # Set defaults
    if(NOT IMPORT_TARGET_NAME)
        set(IMPORT_TARGET_NAME "${LIB_NAME}::${LIB_NAME}")
    endif()

    # Determine library type
    if(IMPORT_STATIC)
        set(LIB_TYPE STATIC)
    elseif(IMPORT_SHARED)
        set(LIB_TYPE SHARED)
    else()
        message(FATAL_ERROR "${LIB_NAME}: Must specify STATIC or SHARED")
    endif()

    # Platform-specific library naming
    if(LIB_TYPE STREQUAL "STATIC")
        if(WIN32)
            set(LIB_FILENAME "${LIB_NAME}.lib")
        else()
            set(LIB_FILENAME "lib${LIB_NAME}.a")
        endif()
    else() # SHARED
        if(WIN32)
            set(LIB_FILENAME "${LIB_NAME}.lib")  # Import library
            set(DLL_FILENAME "${LIB_NAME}.dll")  # Runtime library
        elseif(APPLE)
            set(LIB_FILENAME "lib${LIB_NAME}.dylib")
        else() # Linux
            set(LIB_FILENAME "lib${LIB_NAME}.so")
        endif()
    endif()

    # Install paths
    set(INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/.third_party/${LIB_NAME}")
    set(LIB_FILE "${INSTALL_PREFIX}/lib/${LIB_FILENAME}")
    set(INCLUDE_DIR "${INSTALL_PREFIX}/include")

    # Ensure directories exist (CMake needs them to exist for INTERFACE_INCLUDE_DIRECTORIES)
    file(MAKE_DIRECTORY "${INSTALL_PREFIX}/lib" "${INCLUDE_DIR}" "${INSTALL_PREFIX}/bin")

    # Check if we need to build using git commit hash
    execute_process(
            COMMAND git rev-parse HEAD
            WORKING_DIRECTORY "${IMPORT_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE GIT_RESULT
    )

    if(NOT GIT_RESULT EQUAL 0)
        message(FATAL_ERROR "[${LIB_NAME}] Failed to get git commit hash")
    endif()

    # Simple fingerprint: commit hash + build type + platform
    set(CURRENT_FINGERPRINT "${GIT_COMMIT_HASH}_${CMAKE_BUILD_TYPE}_${CMAKE_SYSTEM_NAME}")
    set(FINGERPRINT_FILE "${INSTALL_PREFIX}/.build_fingerprint")

    set(NEEDS_BUILD FALSE)
    if(NOT EXISTS "${LIB_FILE}")
        set(NEEDS_BUILD TRUE)
        message(STATUS "[${LIB_NAME}] Library not found, will build")
    elseif(EXISTS "${FINGERPRINT_FILE}")
        file(READ "${FINGERPRINT_FILE}" PREVIOUS_FINGERPRINT)
        string(STRIP "${PREVIOUS_FINGERPRINT}" PREVIOUS_FINGERPRINT)
        if(NOT "${CURRENT_FINGERPRINT}" STREQUAL "${PREVIOUS_FINGERPRINT}")
            set(NEEDS_BUILD TRUE)
            message(STATUS "[${LIB_NAME}] Commit/platform changed, will rebuild")
        else()
            message(STATUS "[${LIB_NAME}] Up to date (${GIT_COMMIT_HASH})")
        endif()
    else()
        set(NEEDS_BUILD TRUE)
        message(STATUS "[${LIB_NAME}] First build")
    endif()

    # Build if needed
    if(NEEDS_BUILD)
        ExternalProject_Add(${LIB_NAME}_build
                SOURCE_DIR "${IMPORT_SOURCE_DIR}"
                CMAKE_ARGS
                -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                ${IMPORT_CMAKE_ARGS}
                BUILD_BYPRODUCTS "${LIB_FILE}"
        )

        # Update fingerprint after successful build
        ExternalProject_Add_Step(${LIB_NAME}_build update_fingerprint
                COMMAND ${CMAKE_COMMAND} -E echo "${CURRENT_FINGERPRINT}" > "${FINGERPRINT_FILE}"
                DEPENDEES install
                COMMENT "Updating build fingerprint"
        )

        set(BUILD_TARGET ${LIB_NAME}_build)
    endif()

    # Create imported target
    add_library(${IMPORT_TARGET_NAME} ${LIB_TYPE} IMPORTED GLOBAL)
    set_target_properties(${IMPORT_TARGET_NAME} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
    )

    if(LIB_TYPE STREQUAL "STATIC")
        set_target_properties(${IMPORT_TARGET_NAME} PROPERTIES
                IMPORTED_LOCATION "${LIB_FILE}"
        )
    else()
        # Shared library - platform specific handling
        if(WIN32)
            # Windows: separate import lib and DLL
            set(DLL_FILE "${INSTALL_PREFIX}/bin/${DLL_FILENAME}")
            set_target_properties(${IMPORT_TARGET_NAME} PROPERTIES
                    IMPORTED_IMPLIB "${LIB_FILE}"
                    IMPORTED_LOCATION "${DLL_FILE}"
            )
        else()
            # Linux/macOS: single shared library file
            set_target_properties(${IMPORT_TARGET_NAME} PROPERTIES
                    IMPORTED_LOCATION "${LIB_FILE}"
            )
        endif()
    endif()

    # Add dependency if we're building
    if(NEEDS_BUILD)
        add_dependencies(${IMPORT_TARGET_NAME} ${BUILD_TARGET})
    endif()

endfunction()