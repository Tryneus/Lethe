#ifndef _LETHEFUNCTIONS_H
#define _LETHEFUNCTIONS_H

/*
 * The LetheFunctions.h header provides the common functions implemented
 *  for both platforms.
 */

#include <string>
#include <vector>
#include "LetheTypes.h"

#if defined(__WIN32__) || defined(_WIN32)
  namespace lethe
  {
    // Operator to output a handle in Windows, since the base type is void*
    std::ostream& operator << (std::ostream& out, const Handle& handle);
    void sleep_ms(uint32_t timeout);
  }

#elif defined(__linux__)
  #include <stddef.h>
  #include <stdlib.h>

  namespace lethe
  {
    void sleep_ms(uint32_t timeout);
  }

#else
  #error "Platform not detected"
#endif

namespace lethe
{
  // Returns the number of milliseconds since 12:00 AM Jan 1, 1970
  uint64_t    getTime();
  // Returns the current time of day as a string, format: "Month Day Hours:Minutes:Seconds:Milliseconds"
  std::string getTimeString();

  // Returns the ID of the current process
  uint32_t getProcessId();

  // Returns the ID of the current thread
  uint32_t getThreadId();

  // Seeds the random-number-generator
  uint32_t seedRandom(uint32_t seed = 0);

  class WaitObject;

  // Waits for a single WaitObject to trigger
  WaitResult WaitForObject(WaitObject& obj, uint32_t timeout = INFINITE);
  WaitResult WaitForObject(Handle handle, uint32_t timeout = INFINITE);

  // Returns a string-explanation of the last error to occur from a system call
  std::string lastError();
  // Returns a string-explanation of the specified error code
  std::string getErrorString(uint32_t errorCode);
}

#endif
