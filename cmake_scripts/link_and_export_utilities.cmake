function(add_and_export_library libraryName)
    cmake_parse_arguments(
            LIBRARY_ARGS
            ""              # No options
            "TYPE"          # Single value arguments
            "FILES"         # Multi-value arguments
            ${ARGN}
    )

    set(library_type ${LIBRARY_ARGS_TYPE})
    if(NOT library_type)
        message(WARNING "No library type was provided for ${libraryName}. Defaulting to STATIC.")
        set(library_type STATIC)
    endif()

    if(NOT LIBRARY_ARGS_FILES)
        message(FATAL_ERROR "No files provided for ${libraryName}.")
    endif()

    # Create the library
    add_library(${libraryName} ${library_type} ${LIBRARY_ARGS_FILES})
    # Link with the export header containing the export symbol macro
    target_link_libraries(${libraryName} PUBLIC ExportHeader)
    # Link its include directory
    target_include_directories(${libraryName} PUBLIC .)


    # Add compile definitions if DLL_EXPORT is needed
    if (NEEDS_DLL_EXPORT)
        message(STATUS "DLL_EXPORT defined for ${libraryName}.")
        target_compile_definitions(${libraryName} PRIVATE WIN_EXPORT)
    else()
        message(STATUS "DLL_EXPORT not needed for ${libraryName}.")
    endif()


    # Install the library
    if(NOT library_type STREQUAL "STATIC")
        install(TARGETS ${libraryName}
                RUNTIME DESTINATION ${BINARY_DIRECTORY}       # DLLs
                LIBRARY DESTINATION ${BINARY_DIRECTORY_LIBS}  # .so/.dylib
        )
    endif ()
endfunction()