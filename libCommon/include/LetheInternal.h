#ifndef _LETHEINTERNAL_H
#define _LETHEINTERNAL_H

// Common Lethe stuff to not expose to users

// Compiler-specific stuff
#if defined(__GNUG__)
  #define GCC_UNUSED __attribute__((unused))
#else
  #define GCC_UNUSED
#endif

#endif
