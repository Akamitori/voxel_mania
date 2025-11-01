#pragma once

// Define EXPORTED for any platform
#if defined(_WIN32) || defined(__CYGWIN__)
  // Windows
  #ifdef WIN_EXPORT
    // Exporting (building a DLL)
    #ifdef __GNUC__
      #define EXPORTED __attribute__((dllexport))
    #else
      #define EXPORTED __declspec(dllexport)
    #endif
  #elif defined(WIN_STATIC)
// Building or using a static library → no import/export
    #define EXPORTED
  #else
    // Importing from a DLL
    #ifdef __GNUC__
      #define EXPORTED __attribute__((dllimport))
    #else
      #define EXPORTED __declspec(dllimport)
    #endif
  #endif
  #define NOT_EXPORTED
#else
  // Unix-like systems (Linux, macOS, BSD, etc.)
  #if defined(__GNUC__) && (__GNUC__ >= 4)
    #define EXPORTED __attribute__((visibility("default")))
    #define NOT_EXPORTED __attribute__((visibility("hidden")))
  #else
    #define EXPORTED
    #define NOT_EXPORTED
  #endif
#endif