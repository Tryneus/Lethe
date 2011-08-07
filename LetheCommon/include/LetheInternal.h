#ifndef _LETHEINTERNAL_H
#define _LETHEINTERNAL_H

#include "LetheTypes.h"

// Common Lethe stuff to not expose to users

// Compiler-specific stuff
#if defined(__GNUG__)
  #define GCC_UNUSED __attribute__((unused))
#else
  #define GCC_UNUSED
#endif

namespace lethe
{
  // Returns the absolute end time from the current time based on the timeout (in ms)
  uint64_t getEndTime(uint32_t timeout);
  // Returns the difference between the current time and end time (in ms)
  uint64_t getTimeout(uint64_t endTime);
}

#endif
