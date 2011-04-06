#ifndef _LETHECOMPLEX_H
#define _LETHECOMPLEX_H

/*
 * The LetheComplex.h header is used for complex types that utilize basic
 *  types. At the moment, this is only threads.
 */


#if defined(__WIN32__) || defined(_WIN32)

  namespace lethe
  {
    class WindowsThread;

    typedef WindowsThread Thread;
  }

  #include "windows/WindowsThread.h"

#elif defined(__linux__)

  namespace lethe
  {
    class LinuxThread;

    typedef LinuxThread Thread;
  }

  #include "linux/LinuxThread.h"

#else
  #error "Platform not detected"
#endif

#endif
