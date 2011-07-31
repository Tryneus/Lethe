#ifndef _LETHEBASIC_H
#define _LETHEBASIC_H

/*
 * The LetheBasic.h header includes and typedefs basic objects for
 *  use by users or the more complex objects in LetheComplex.h or
 *  the functions in LetheFunctions.h.
 */

#include "WaitObject.h"
#include "ByteStream.h"
#include "MessageStream.h"


#if defined(__WIN32__) || defined(_WIN32)

  namespace lethe
  {
    class WindowsWaitSet;
    typedef WindowsWaitSet WaitSet;

    class WindowsEvent;
    typedef WindowsEvent Event;

    class WindowsMutex;
    typedef WindowsMutex Mutex;

    class WindowsSemaphore;
    typedef WindowsSemaphore Semaphore;

    class WindowsPipe;
    typedef WindowsPipe Pipe;

    class WindowsTimer;
    typedef WindowsTimer Timer;

    class WindowsSharedMemory;
    typedef WindowsSharedMemory SharedMemory;

    class WindowsAtomic32;
    typedef WindowsAtomic32 Atomic32;
    class WindowsAtomic64;
    typedef WindowsAtomic64 Atomic64;
  }

  #include "windows/WindowsWaitSet.h"
  #include "windows/WindowsEvent.h"
  #include "windows/WindowsMutex.h"
  #include "windows/WindowsSemaphore.h"
  #include "windows/WindowsPipe.h"
  #include "windows/WindowsTimer.h"
  #include "windows/WindowsSharedMemory.h"
  #include "windows/WindowsAtomic.h"

  namespace lethe
  {
    typedef WindowsAtomic Atomic;
  }

#elif defined(__linux__)

  namespace lethe
  {
    class LinuxWaitSet;
    typedef LinuxWaitSet WaitSet;

    class LinuxEvent;
    typedef LinuxEvent Event;

    class LinuxMutex;
    typedef LinuxMutex Mutex;

    class LinuxSemaphore;
    typedef LinuxSemaphore Semaphore;

    class LinuxPipe;
    typedef LinuxPipe Pipe;

    class LinuxTimer;
    typedef LinuxTimer Timer;

    class LinuxSharedMemory;
    typedef LinuxSharedMemory SharedMemory;

    class LinuxAtomic32;
    class LinuxAtomic64;

    class LinuxAtomic32;
    typedef LinuxAtomic32 Atomic32;
    class LinuxAtomic64;
    typedef LinuxAtomic64 Atomic64;
  }

  #include "linux/LinuxWaitSet.h"
  #include "linux/LinuxEvent.h"
  #include "linux/LinuxMutex.h"
  #include "linux/LinuxSemaphore.h"
  #include "linux/LinuxPipe.h"
  #include "linux/LinuxTimer.h"
  #include "linux/LinuxSharedMemory.h"
  #include "linux/LinuxAtomic.h"

  namespace lethe
  {
    typedef LinuxAtomic Atomic;
  }

#else
  #error "Platform not detected"
#endif

#endif
