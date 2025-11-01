function(add_and_export_library libraryName)
    cmake_parse_arguments(LIB
            ""              # No options
            "TYPE"          # Single value arguments  
            "FILES"         # Multi-value arguments
            ${ARGN}
    )

    # Validate inputs
    if (NOT LIB_FILES)
        message(FATAL_ERROR "No files provided for ${libraryName}")
    endif ()

    if (NOT LIB_TYPE)
        set(LIB_TYPE STATIC)
        message(STATUS "${libraryName}: No type specified, defaulting to STATIC")
    endif ()

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
    # Handle DLL exports for static libraries on Windows
    elseif(LIB_TYPE STREQUAL "STATIC" AND WIN32)
        target_compile_definitions(${libraryName} PUBLIC WIN_STATIC)
    endif()

    # Optional: Set up proper target properties for better IDE support
    set_target_properties(${libraryName} PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN YES
            POSITION_INDEPENDENT_CODE ON
    )

endfunction()