function(add_and_export_library libraryName)
    cmake_parse_arguments(LIB
            ""
            "TYPE;C_STANDARD;CXX_STANDARD"
            "FILES;PROPERTIES"
            ${ARGN}
    )

    if (LIB_TYPE STREQUAL "INTERFACE")
        # INTERFACE libraries don't take files
        add_library(${libraryName} INTERFACE)
        target_include_directories(${libraryName} INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:include>
        )
        target_link_libraries(${libraryName} INTERFACE ExportHeader)
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

        # Set up include directories and export header
        target_include_directories(${libraryName} PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:include>
        )
        target_link_libraries(${libraryName} PUBLIC ExportHeader)

        # Handle DLL exports for shared libraries on Windows
        if (LIB_TYPE STREQUAL "SHARED" AND WIN32)
            target_compile_definitions(${libraryName} PRIVATE WIN_EXPORT)
        elseif(LIB_TYPE STREQUAL "STATIC" AND WIN32)
            target_compile_definitions(${libraryName} PUBLIC WIN_STATIC)
        endif()

        # Set default target properties
        set_target_properties(${libraryName} PROPERTIES
                CXX_VISIBILITY_PRESET hidden
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