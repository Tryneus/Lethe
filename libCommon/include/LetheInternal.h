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
  uint32_t getEndTime(uint32_t timeout);
  uint32_t getTimeout(uint32_t endTime);
}

#endif
