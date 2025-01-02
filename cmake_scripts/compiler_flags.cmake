# Detect the compiler and set CMake cache variables

# Detect if a DLL export attribute is needed
if (MSVC)
    set(NEEDS_DLL_EXPORT ON CACHE BOOL "MSVC requires dllexport")
    message(STATUS "Detected MSVC. Set NEEDS_DLL_EXPORT=ON")
elseif (CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")
    set(NEEDS_DLL_EXPORT ON CACHE BOOL "Cygwin requires dllexport")
    message(STATUS "Detected Cygwin. Set NEEDS_DLL_EXPORT=ON")
else()
    set(NEEDS_DLL_EXPORT OFF CACHE BOOL "Other platforms do not require dllexport")
    message(STATUS "Detected other platform/compiler. Set NEEDS_DLL_EXPORT=OFF")
endif()