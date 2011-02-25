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
    std::ostream& operator << (std::ostream& out, const Handle& handle);
  }

#elif defined(__linux__)
  #include <stddef.h>
  #include <stdlib.h>

  namespace lethe
  {
    void Sleep(uint32_t timeout);
  }

#else
  #error "Platform not detected"
#endif

namespace lethe
{
  uint64_t    getTime();
  std::string getTimeString();

  uint32_t getProcessId();
  // TODO: uint32_t getThreadId();

  uint32_t seedRandom(uint32_t seed = 0);

  class WaitObject;

  WaitResult WaitForObject(WaitObject& obj, uint32_t timeout = INFINITE);

  std::string lastError();
}

#endif
