#ifndef _LETHETYPES_H
#define _LETHETYPES_H

/**
 * The LetheTypes.h header defines non-object types used by more complex
 *  objects in the library.
 */

#if defined(__WIN32__) || defined(_WIN32)

  #if defined(_MSC_VER)
    #include <new.h>    // Include placement new for Visual C++
    #include "stdint.h" // Include local stdint.h when using Visual C++
  #else
    #include <stdint.h>
  #endif

  #include <Windows.h>

  namespace lethe
  {
    typedef HANDLE Handle;
  }

#elif defined(__linux__)

  #include <stdint.h>

  namespace lethe
  {
    typedef int Handle;
  }

  // Some defines to copy identifiers used in WIN32
  #define INVALID_HANDLE_VALUE -1
  #define INFINITE -1

#else
  #error "Platform not detected"
#endif

// Compiler-specific stuff : TODO: don't expose this to users?
#if defined(__GNUG__)
  #define GCC_UNUSED __attribute__((unused))
#else
  #define GCC_UNUSED
#endif

namespace lethe
{
  enum WaitResult
  {
    WaitSuccess = 0,
    WaitError = 1,
    WaitAbandoned = -2,
    WaitTimeout = -3
  };
}

#endif
