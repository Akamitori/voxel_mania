include_guard(GLOBAL)
include(GenerateExportHeader)

# Warning level for OUR targets only (not 3rd party code). Warnings, not errors.
option(VOXEL_WARNINGS "Enable -Wall -Wextra (/W4 on MSVC) for the project's own targets" ON)
function(voxel_enable_warnings target)
    if (NOT VOXEL_WARNINGS)
        return()
    endif ()
    if (MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else ()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endif ()
endfunction()

function(add_and_export_library libraryName)
    cmake_parse_arguments(LIB
            "NO_INSTALL"
            "TYPE;C_STANDARD;CXX_STANDARD"
            "FILES;PROPERTIES"
            ${ARGN}
    )

    if (LIB_TYPE STREQUAL "INTERFACE")
        # INTERFACE libraries don't take files
        add_library(${libraryName} INTERFACE)
        target_include_directories(${libraryName} INTERFACE
                ${CMAKE_CURRENT_SOURCE_DIR}
        )
    else()
        # Validate inputs for non-INTERFACE libraries
        if (NOT LIB_FILES)
            message(FATAL_ERROR "No files provided for ${libraryName}")
        endif()

        if (NOT LIB_TYPE)
            set(LIB_TYPE STATIC)
            message(STATUS "${libraryName}: No type specified, defaulting to STATIC")
        endif()

        # Infer linker language from which standard is set
        if (LIB_C_STANDARD AND NOT LIB_CXX_STANDARD)
            # Only C_STANDARD set → Pure C library
            set(LINKER_LANG C)
        else()
            # CXX_STANDARD set, OR both set, OR neither set → Use C++
            set(LINKER_LANG CXX)
        endif()

        # Create the library
        add_library(${libraryName} ${LIB_TYPE} ${LIB_FILES})
        voxel_enable_warnings(${libraryName})

        # Per-library export header: generates <lowercase name>_export.h in the binary dir,
        # defining <UPPERCASE NAME>_EXPORT / _NO_EXPORT / _DEPRECATED.
        # For SHARED libs CMake defines <name>_EXPORTS only while compiling this library,
        # so the macro is dllexport here and dllimport for every consumer.
        generate_export_header(${libraryName})
        string(TOUPPER "${libraryName}" LIB_UPPER_NAME)

        target_include_directories(${libraryName} PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_BINARY_DIR}
        )

        # Shared libs are shipped next to the executable on install (see the top-level CMakeLists).
        # NO_INSTALL: test/benchmark-only libraries.
        if (LIB_TYPE STREQUAL "SHARED" AND NOT LIB_NO_INSTALL)
            set_property(GLOBAL APPEND PROPERTY VOXEL_SHARED_LIBS ${libraryName})
        endif()

        # Static libs (and their consumers) must see the export macro as empty
        if (LIB_TYPE STREQUAL "STATIC")
            target_compile_definitions(${libraryName} PUBLIC ${LIB_UPPER_NAME}_STATIC_DEFINE)
        endif()

        # Set default target properties
        set_target_properties(${libraryName} PROPERTIES
                CXX_VISIBILITY_PRESET hidden
                C_VISIBILITY_PRESET hidden
                VISIBILITY_INLINES_HIDDEN YES
                POSITION_INDEPENDENT_CODE ON
                LINKER_LANGUAGE ${LINKER_LANG}
        )

        # Set language standards if provided
        if (LIB_C_STANDARD)
            set_target_properties(${libraryName} PROPERTIES
                    C_STANDARD ${LIB_C_STANDARD}
                    C_STANDARD_REQUIRED ON
            )
        endif()

        if (LIB_CXX_STANDARD)
            set_target_properties(${libraryName} PROPERTIES
                    CXX_STANDARD ${LIB_CXX_STANDARD}
                    CXX_STANDARD_REQUIRED ON
            )
        endif()

        # Apply additional custom properties if provided
        if (LIB_PROPERTIES)
            set_target_properties(${libraryName} PROPERTIES ${LIB_PROPERTIES})
        endif()
    endif()
endfunction()