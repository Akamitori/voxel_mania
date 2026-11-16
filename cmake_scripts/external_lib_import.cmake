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

    # Same git as the submodule step (find_package(Git)); plain "git" may not be on PATH on Windows
    if (NOT GIT_EXECUTABLE)
        find_package(Git QUIET)
    endif ()
    execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
            WORKING_DIRECTORY "${IMPORT_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE GIT_RESULT
            ERROR_QUIET
    )

    if (NOT GIT_RESULT EQUAL 0)
        # Fixed value (not a timestamp): a timestamp would create a new cache dir on every configure
        set(GIT_COMMIT_HASH "nogit")
        message(WARNING "[${LIB_NAME}] Git not available, source changes won't be detected; delete .prebuild_libs/${LIB_NAME} to force a rebuild")
    endif ()
    # Current source commits, used by the prune_prebuilt_libs target
    set_property(GLOBAL APPEND PROPERTY VOXEL_PREBUILT_COMMITS ${GIT_COMMIT_HASH})

    # Configure arguments that don't depend on where the library is installed.
    # These (plus commit and compiler) select the cache dir, so they must not contain INSTALL_PREFIX.
    set(EP_KEY_ARGS
            # Pin the install layout: GNUInstallDirs defaults to lib64 on Fedora/RHEL/SUSE,
            # but LIB_FILE below expects lib/.
            -DCMAKE_INSTALL_LIBDIR=lib
            -DCMAKE_INSTALL_BINDIR=bin
            -DCMAKE_INSTALL_INCLUDEDIR=include
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
            -DCMAKE_CXX_STANDARD_REQUIRED=ON
            ${IMPORT_CMAKE_ARGS}
    )
    if (CMAKE_TOOLCHAIN_FILE)
        list(APPEND EP_KEY_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
    endif ()
    # Same linker as the main project (set by cmake_scripts/linker_setup.cmake). Part of the
    # fingerprint, so switching linkers rebuilds the cached libraries.
    list(APPEND EP_KEY_ARGS ${VOXEL_LINKER_FORWARD_ARGS})

    # Fingerprint -> cache dir. Every distinct setup (submodule commit, OS, compiler path/version,
    # build type, configure args) gets its own dir under .prebuild_libs/<lib>/, so build dirs with
    # different compilers/types share the cache without wiping each other's libraries.
    if (CMAKE_BUILD_TYPE)
        set(CONFIG_LABEL "${CMAKE_BUILD_TYPE}")
    else ()
        set(CONFIG_LABEL "NoConfig")
    endif ()
    set(CURRENT_FINGERPRINT
            "commit=${GIT_COMMIT_HASH}\nconfig=${CONFIG_LABEL}\nsystem=${CMAKE_SYSTEM_NAME}\ncompiler=${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}\nargs=${EP_KEY_ARGS}\n")
    string(SHA1 FINGERPRINT_HASH "${CURRENT_FINGERPRINT}")
    string(SUBSTRING "${FINGERPRINT_HASH}" 0 10 FINGERPRINT_HASH)

    set(INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/.prebuild_libs/${LIB_NAME}/${CONFIG_LABEL}-${CMAKE_CXX_COMPILER_ID}-${FINGERPRINT_HASH}")
    set(LIB_FILE "${INSTALL_PREFIX}/lib/${LIB_FILENAME}")
    set(INCLUDE_DIR "${INSTALL_PREFIX}/include")
    # Written only after a successful install, so it marks the cache dir as complete
    set(FINGERPRINT_FILE "${INSTALL_PREFIX}/.build_fingerprint")
    # ExternalProject_Add default PREFIX: sub-build tree, stamps and its CMakeCache.txt
    set(EP_PREFIX_DIR "${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}_build-prefix")

    set(EP_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ${EP_KEY_ARGS})
    if (UNIX)
        if (APPLE)
            set(RPATH_ORIGIN "@loader_path")
        else ()
            set(RPATH_ORIGIN "$ORIGIN")
        endif ()

        # '|' is turned back into ';' by ExternalProject (LIST_SEPARATOR below). A literal ';'
        # would split this into two list elements and silently drop the second RPATH entry.
        list(APPEND EP_CMAKE_ARGS
                -DCMAKE_BUILD_RPATH=${INSTALL_PREFIX}/lib
                "-DCMAKE_INSTALL_RPATH=${RPATH_ORIGIN}|${RPATH_ORIGIN}/../lib"
                -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE
        )
    endif ()

    # Build only if this cache dir isn't complete. An incomplete dir (interrupted or failed
    # build) is wiped, along with this build dir's sub-build tree, so the rebuild starts clean.
    if (EXISTS "${LIB_FILE}" AND EXISTS "${FINGERPRINT_FILE}")
        set(NEEDS_BUILD FALSE)
        message(STATUS "[${LIB_NAME}] ✓ Using cache ${CONFIG_LABEL}-${CMAKE_CXX_COMPILER_ID}-${FINGERPRINT_HASH}")
    else ()
        set(NEEDS_BUILD TRUE)
        file(REMOVE_RECURSE "${INSTALL_PREFIX}" "${EP_PREFIX_DIR}")
        message(STATUS "[${LIB_NAME}] No complete cache for this setup, will build into ${CONFIG_LABEL}-${CMAKE_CXX_COMPILER_ID}-${FINGERPRINT_HASH}")
    endif ()
    file(MAKE_DIRECTORY "${INSTALL_PREFIX}/lib" "${INCLUDE_DIR}" "${INSTALL_PREFIX}/bin")

    # The build-or-reuse decision above is made at configure time. Make the cached files
    # configure dependencies so that deleting them (e.g. removing .prebuild_libs) re-runs
    # CMake on the next build, which then schedules the rebuild, instead of the build
    # failing on a missing library.
    if (NOT NEEDS_BUILD)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${LIB_FILE}" "${FINGERPRINT_FILE}")
    endif ()

    if (LIB_TYPE STREQUAL "SHARED" AND WIN32)
        set(DLL_FILE "${INSTALL_PREFIX}/bin/${DLL_FILENAME}")
    endif ()

    # Per-build-dir copy of the cached lib/ and bin/ (see mirror_prebuilt.cmake). The imported
    # target points here, and only these copies are declared as build outputs, so Clean / Rebuild
    # deletes the copy instead of files in the shared .prebuild_libs cache.
    set(LOCAL_PREFIX "${CMAKE_BINARY_DIR}/prebuilt/${LIB_NAME}")
    set(LOCAL_LIB_FILE "${LOCAL_PREFIX}/lib/${LIB_FILENAME}")
    set(LOCAL_OUTPUTS "${LOCAL_LIB_FILE}")
    if (DEFINED DLL_FILE)
        set(LOCAL_DLL_FILE "${LOCAL_PREFIX}/bin/${DLL_FILENAME}")
        list(APPEND LOCAL_OUTPUTS "${LOCAL_DLL_FILE}")
    endif ()
    set(MIRROR_STAMP "${LOCAL_PREFIX}/.mirrored")
    set(MIRROR_COMMAND ${CMAKE_COMMAND}
            -DSRC=${INSTALL_PREFIX} -DDST=${LOCAL_PREFIX} -DSTAMP=${MIRROR_STAMP}
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/mirror_prebuilt.cmake")

    # Only create ExternalProject if we actually need to build
    if (NEEDS_BUILD)
        ExternalProject_Add(${LIB_NAME}_build
                SOURCE_DIR "${IMPORT_SOURCE_DIR}"
                # No explicit CMAKE_GENERATOR: left out, ExternalProject uses the parent's
                # generator AND inherits its platform/toolset (-A/-T); passing it drops those.
                LIST_SEPARATOR |
                CMAKE_ARGS ${EP_CMAKE_ARGS}
                BUILD_ALWAYS 0
                # Run the sub-build in Ninja's console pool: SDL3 and assimp then build one at a time
                # (each with full parallelism) instead of all at once on top of the main build.
                USES_TERMINAL_BUILD TRUE
                UPDATE_COMMAND ""
                DOWNLOAD_COMMAND ""
        )

        # Update fingerprint after successful build: the value is written at configure time
        # and copied into the install prefix only once install succeeded (no shell redirection).
        # copy_if_different: re-running this step (e.g. after Clean) must not touch the cached file,
        # since build dirs reusing this cache depend on it and would reconfigure and re-copy.
        set(PENDING_FINGERPRINT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}.fingerprint")
        file(WRITE "${PENDING_FINGERPRINT_FILE}" "${CURRENT_FINGERPRINT}")
        ExternalProject_Add_Step(${LIB_NAME}_build update_fingerprint
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PENDING_FINGERPRINT_FILE}" "${FINGERPRINT_FILE}"
                DEPENDEES install
                COMMENT "Updating build fingerprint for ${LIB_NAME}"
        )
        ExternalProject_Add_Step(${LIB_NAME}_build mirror_to_build_dir
                COMMAND ${MIRROR_COMMAND}
                DEPENDEES update_fingerprint
                BYPRODUCTS ${LOCAL_OUTPUTS} ${MIRROR_STAMP}
                COMMENT "Copying ${LIB_NAME} into the build dir"
        )
    else()
        # Cache already complete: just (re)copy it into this build dir when it changes
        add_custom_command(
                OUTPUT ${MIRROR_STAMP}
                BYPRODUCTS ${LOCAL_OUTPUTS}
                COMMAND ${MIRROR_COMMAND}
                DEPENDS "${FINGERPRINT_FILE}" "${LIB_FILE}"
                COMMENT "Copying cached ${LIB_NAME} into the build dir"
                VERBATIM
        )
        add_custom_target(${LIB_NAME}_build DEPENDS ${MIRROR_STAMP})
    endif ()


    # Create imported target AFTER ExternalProject
    add_library(${IMPORT_TARGET_NAME} ${LIB_TYPE} IMPORTED GLOBAL)
    set_target_properties(${IMPORT_TARGET_NAME} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
    )

    if (LIB_TYPE STREQUAL "STATIC")
        set_target_properties(${IMPORT_TARGET_NAME} PROPERTIES
                IMPORTED_LOCATION "${LOCAL_LIB_FILE}"
        )
    else ()
        if (WIN32)
            set_target_properties(${IMPORT_TARGET_NAME} PROPERTIES
                    IMPORTED_IMPLIB "${LOCAL_LIB_FILE}"
                    IMPORTED_LOCATION "${LOCAL_DLL_FILE}"
            )
        else ()
            set_target_properties(${IMPORT_TARGET_NAME} PROPERTIES
                    IMPORTED_LOCATION "${LOCAL_LIB_FILE}"
            )
        endif ()
    endif ()

    # Critical: Add dependency so target knows to wait for build
    add_dependencies(${IMPORT_TARGET_NAME} ${LIB_NAME}_build)

endfunction()