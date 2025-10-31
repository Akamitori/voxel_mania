include(ExternalProject)

function(import_external_library LIB_NAME)
    set(options STATIC SHARED)
    set(oneValueArgs SOURCE_DIR TARGET_NAME)
    set(multiValueArgs CMAKE_ARGS)
    cmake_parse_arguments(IMPORT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Auto-detect SOURCE_DIR from current directory context
    if (NOT IMPORT_SOURCE_DIR)
        set(IMPORT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${LIB_NAME}")

        if (NOT EXISTS "${IMPORT_SOURCE_DIR}/CMakeLists.txt")
            message(FATAL_ERROR "${LIB_NAME}: No CMakeLists.txt found at ${IMPORT_SOURCE_DIR}. Please specify SOURCE_DIR explicitly.")
        endif()

        message(STATUS "[${LIB_NAME}] Using source: ${IMPORT_SOURCE_DIR}")
    endif()

    if (NOT IMPORT_TARGET_NAME)
        set(IMPORT_TARGET_NAME "${LIB_NAME}::${LIB_NAME}")
    endif ()

    # Determine library type
    if (IMPORT_STATIC)
        set(LIB_TYPE STATIC)
    elseif (IMPORT_SHARED)
        set(LIB_TYPE SHARED)
    else ()
        message(FATAL_ERROR "${LIB_NAME}: Must specify STATIC or SHARED")
    endif ()

    # Platform-specific library naming
    if (LIB_TYPE STREQUAL "STATIC")
        if (MSVC)
            set(LIB_FILENAME "${LIB_NAME}.lib")
        else ()
            set(LIB_FILENAME "lib${LIB_NAME}.a")
        endif ()
    else () # SHARED
        if (MSVC)
            set(LIB_FILENAME "${LIB_NAME}.lib")
            set(DLL_FILENAME "${LIB_NAME}.dll")
        elseif (MINGW)
            set(LIB_FILENAME "lib${LIB_NAME}.dll.a")
            set(DLL_FILENAME "${LIB_NAME}.dll")
        elseif (WIN32)
            # Fallback for other Windows compilers (clang-cl, etc.)
            set(LIB_FILENAME "${LIB_NAME}.lib")
            set(DLL_FILENAME "${LIB_NAME}.dll")
        elseif (APPLE)
            set(LIB_FILENAME "lib${LIB_NAME}.dylib")
        else ()
            # Linux and other Unix-like systems
            set(LIB_FILENAME "lib${LIB_NAME}.so")
        endif ()
    endif ()

    # Install paths
    set(INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/.prebuild_libs/${LIB_NAME}/${CMAKE_BUILD_TYPE}")
    set(LIB_FILE "${INSTALL_PREFIX}/lib/${LIB_FILENAME}")
    set(INCLUDE_DIR "${INSTALL_PREFIX}/include")

    # Ensure directories exist
    file(MAKE_DIRECTORY "${INSTALL_PREFIX}/lib" "${INCLUDE_DIR}" "${INSTALL_PREFIX}/bin")

    execute_process(
            COMMAND git rev-parse HEAD
            WORKING_DIRECTORY "${IMPORT_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE GIT_RESULT
            ERROR_QUIET 
    )

    if (NOT GIT_RESULT EQUAL 0)
        # Fallback: use timestamp or fixed value
        string(TIMESTAMP GIT_COMMIT_HASH "%Y%m%d%H%M%S")
        message(WARNING "[${LIB_NAME}] Git not available, using timestamp: ${GIT_COMMIT_HASH}")
    endif ()

    # Fingerprint
    set(CURRENT_FINGERPRINT "${GIT_COMMIT_HASH}_${CMAKE_BUILD_TYPE}_${CMAKE_SYSTEM_NAME}_${CMAKE_CXX_COMPILER_ID}")
    set(FINGERPRINT_FILE "${INSTALL_PREFIX}/.build_fingerprint")
    
    # Determine if build is needed
    set(NEEDS_BUILD FALSE)
    if (NOT EXISTS "${LIB_FILE}")
        set(NEEDS_BUILD TRUE)
        message(STATUS "[${LIB_NAME}] Library not found, will build")
    elseif (EXISTS "${FINGERPRINT_FILE}")
        file(READ "${FINGERPRINT_FILE}" PREVIOUS_FINGERPRINT)
        string(STRIP "${PREVIOUS_FINGERPRINT}" PREVIOUS_FINGERPRINT)
        if (NOT "${CURRENT_FINGERPRINT}" STREQUAL "${PREVIOUS_FINGERPRINT}")
            set(NEEDS_BUILD TRUE)
            message(STATUS "[${LIB_NAME}] Commit/platform changed (${CURRENT_FINGERPRINT} vs ${PREVIOUS_FINGERPRINT}), will rebuild")
            
            message(STATUS "[${LIB_NAME}] Fingerprint changed, cleaning and rebuilding")
            message(STATUS "[${LIB_NAME}]   Old: ${PREVIOUS_FINGERPRINT}")
            message(STATUS "[${LIB_NAME}]   New: ${CURRENT_FINGERPRINT}")

            # Delete everything to ensure clean rebuild
            file(REMOVE_RECURSE "${INSTALL_PREFIX}")
            file(MAKE_DIRECTORY "${INSTALL_PREFIX}/lib" "${INCLUDE_DIR}" "${INSTALL_PREFIX}/bin")
        else ()
            message(STATUS "[${LIB_NAME}] ✓ Up to date (${GIT_COMMIT_HASH})")
        endif ()
    else ()
        set(NEEDS_BUILD TRUE)
        # Delete everything to ensure clean rebuild
        file(REMOVE_RECURSE "${INSTALL_PREFIX}")
        file(MAKE_DIRECTORY "${INSTALL_PREFIX}/lib" "${INCLUDE_DIR}" "${INSTALL_PREFIX}/bin")
        message(STATUS "[${LIB_NAME}] First build")
    endif ()

    # Collect all byproducts
    set(BUILD_BYPRODUCTS "${LIB_FILE}")
    if (LIB_TYPE STREQUAL "SHARED" AND WIN32)
        set(DLL_FILE "${INSTALL_PREFIX}/bin/${DLL_FILENAME}")
        list(APPEND BUILD_BYPRODUCTS "${DLL_FILE}")
    endif ()

    if (UNIX)
        if (APPLE)
            set(RPATH_ORIGIN "@loader_path")
        else ()
            set(RPATH_ORIGIN "$ORIGIN")
        endif ()

        list(APPEND IMPORT_CMAKE_ARGS
                -DCMAKE_BUILD_RPATH=${INSTALL_PREFIX}/lib
                "-DCMAKE_INSTALL_RPATH=${RPATH_ORIGIN};${RPATH_ORIGIN}/../lib"  # Fixed separator
                -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE
        )
    endif ()

    # Toolchain propagation (optional but recommended)
    if (CMAKE_TOOLCHAIN_FILE)
        list(APPEND IMPORT_CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
    endif ()

    # Always create ExternalProject (even if not building)
    # This ensures proper dependency tracking
    ExternalProject_Add(${LIB_NAME}_build
            SOURCE_DIR "${IMPORT_SOURCE_DIR}"
            CMAKE_GENERATOR ${CMAKE_GENERATOR}
            CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}     
            -DCMAKE_CXX_STANDARD_REQUIRED=ON               
            ${IMPORT_CMAKE_ARGS}
            BUILD_BYPRODUCTS ${BUILD_BYPRODUCTS}
            BUILD_ALWAYS 0
            UPDATE_COMMAND ""
            DOWNLOAD_COMMAND ""
            # Key fix: Skip build/install if not needed
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping build - library up to date"
            INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install - library up to date"
    )

    # Override build/install commands only if we need to build
    if (NEEDS_BUILD)
        ExternalProject_Add_Step(${LIB_NAME}_build force_build
                COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR>
                --config ${CMAKE_BUILD_TYPE}
                --parallel
                COMMENT "Building ${LIB_NAME}"
                DEPENDEES configure
                DEPENDERS build
        )

        ExternalProject_Add_Step(${LIB_NAME}_build force_install
                COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR>
                --config ${CMAKE_BUILD_TYPE}
                --parallel
                --target install
                COMMENT "Installing ${LIB_NAME}"
                DEPENDEES build
                DEPENDERS install
        )

        # Update fingerprint after successful build
        ExternalProject_Add_Step(${LIB_NAME}_build update_fingerprint
                COMMAND ${CMAKE_COMMAND} -E echo "${CURRENT_FINGERPRINT}" > "${FINGERPRINT_FILE}"
                DEPENDEES install
                COMMENT "Updating build fingerprint for ${LIB_NAME}"
        )
    endif ()

    # Create imported target AFTER ExternalProject
    add_library(${IMPORT_TARGET_NAME} ${LIB_TYPE} IMPORTED GLOBAL)
    set_target_properties(${IMPORT_TARGET_NAME} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
    )

    if (LIB_TYPE STREQUAL "STATIC")
        set_target_properties(${IMPORT_TARGET_NAME} PROPERTIES
                IMPORTED_LOCATION "${LIB_FILE}"
        )
    else ()
        if (WIN32)
            set_target_properties(${IMPORT_TARGET_NAME} PROPERTIES
                    IMPORTED_IMPLIB "${LIB_FILE}"
                    IMPORTED_LOCATION "${DLL_FILE}"
            )
        else ()
            set_target_properties(${IMPORT_TARGET_NAME} PROPERTIES
                    IMPORTED_LOCATION "${LIB_FILE}"
            )
        endif ()
    endif ()

    # Critical: Add dependency so target knows to wait for build
    add_dependencies(${IMPORT_TARGET_NAME} ${LIB_NAME}_build)

endfunction()