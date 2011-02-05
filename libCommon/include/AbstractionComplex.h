#ifndef _ABSTRACTIONCOMPLEX_H
#define _ABSTRACTIONCOMPLEX_H

/*
 * The AbstractionComplex.h header is used for complex types that utilize basic
 *  types. At the moment, this is only threads.
 */

#if defined(__WIN32__) || defined(_WIN32)

  class WindowsThread;

  typedef WindowsThread Thread;

  #include "windows/WindowsThread.h"

#elif defined(__linux__)

  class LinuxThread;

  typedef LinuxThread Thread;

  #include "linux/LinuxThread.h"

#else
  #error "Platform not detected"
#endif

#endif
