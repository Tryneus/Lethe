#ifndef _ABSTRACTIONTYPES_H
#define _ABSTRACTIONTYPES_H

/**
 * The AbstractionTypes.h header defines non-object types used by more complex
 *  objects in the library.
 */

// Platform specific stuff
#if defined(__WIN32__) || defined(_WIN32)

  #if defined(_MSC_VER)
    #include <new.h>    // Include placement new for Visual C++
    #include "stdint.h" // Include local stdint.h when using Visual C++
  #else
    #include <stdint.h>
  #endif

  #include <Windows.h>
  typedef HANDLE Handle;

#elif defined(__linux__)

  #include <stdint.h>
  typedef int Handle;

  // Some defines to copy identifiers used in WIN32
  #define INVALID_HANDLE_VALUE -1
  #define INFINITE -1

#else
  #error "Platform not detected"
#endif

// Compiler-specific stuff
#if defined(__GNUG__)
  #define GCC_UNUSED __attribute__((unused))
#else
  #define GCC_UNUSED
#endif

enum WaitResult
{
  WaitSuccess = 0,
  WaitError = 1,
  WaitAbandoned = -2,
  WaitTimeout = -3
};

#endif
