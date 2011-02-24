#ifndef _ABSTRACTIONFUNCTIONS_H
#define _ABSTRACTIONFUNCTIONS_H

/*
 * The AbstractionFunctions.h header provides the common functions implemented
 *  for both platforms.
 */

#include <string>
#include <vector>
#include "AbstractionTypes.h"
#include "WaitObject.h"

#if defined(__WIN32__) || defined(_WIN32)
  std::ostream& operator << (std::ostream& out, const Handle& handle);

#elif defined(__linux__)
  #include <stddef.h>
  #include <stdlib.h>

  void Sleep(uint32_t timeout);

#else
  #error "Platform not detected"
#endif

uint64_t    getTime();
std::string getTimeString();

uint32_t getProcessId();

uint32_t seedRandom(uint32_t seed = 0);

WaitResult WaitForObject(WaitObject& obj, uint32_t timeout = INFINITE);

std::string lastError();

#endif
