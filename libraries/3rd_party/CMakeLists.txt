# Ensure all submodules are initialized
message(STATUS "Initializing git submodules...")
execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE result
)
if (NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to initialize submodules. Please run:\n    git submodule update --init --recursive")
endif ()

include(external_lib_import.cmake)

import_external_library(SDL3
        SHARED
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/libraries/3rd_party/SDL3"
        INSTALL_SUBDIR "SDL3"
        LIB_FILENAME "SDL3.lib"         # Import library (used at link time)
        DLL_FILENAME "SDL3.dll"         # Runtime DLL (used at runtime)
)


## -----glew specific flags----
set(glew-cmake_BUILD_SHARED ON CACHE BOOL "Build the shared glew library")
set(glew-cmake_BUILD_STATIC OFF CACHE BOOL "Build the static glew library")
set(ONLY_LIBS ON CACHE BOOL "Do not build executables")
add_subdirectory(glew)
## -----glew specific flags----

install(TARGETS libglew_shared
        RUNTIME DESTINATION ${BINARY_DIRECTORY}  # DLLs
        LIBRARY DESTINATION ${BINARY_DIRECTORY_LIBS}  # .so/.dylib
)

## run the imgui build
include(imgui_build.cmake)