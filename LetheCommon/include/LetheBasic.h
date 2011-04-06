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
    class WindowsEvent;
    class WindowsMutex;
    class WindowsSemaphore;
    class WindowsPipe;
    class WindowsTimer;
    class WindowsSharedMemory;

    typedef WindowsWaitSet WaitSet;
    typedef WindowsEvent Event;
    typedef WindowsMutex Mutex;
    typedef WindowsSemaphore Semaphore;
    typedef WindowsPipe Pipe;
    typedef WindowsTimer Timer;
    typedef WindowsSharedMemory SharedMemory;
  }

  #include "windows/WindowsWaitSet.h"
  #include "windows/WindowsEvent.h"
  #include "windows/WindowsMutex.h"
  #include "windows/WindowsSemaphore.h"
  #include "windows/WindowsPipe.h"
  #include "windows/WindowsTimer.h"
  #include "windows/WindowsSharedMemory.h"

#elif defined(__linux__)

  namespace lethe
  {
    class LinuxWaitSet;
    class LinuxEvent;
    class LinuxMutex;
    class LinuxSemaphore;
    class LinuxPipe;
    class LinuxTimer;
    class LinuxSharedMemory;

    typedef LinuxWaitSet WaitSet;
    typedef LinuxEvent Event;
    typedef LinuxMutex Mutex;
    typedef LinuxSemaphore Semaphore;
    typedef LinuxPipe Pipe;
    typedef LinuxTimer Timer;
    typedef LinuxSharedMemory SharedMemory;
  }

  #include "linux/LinuxWaitSet.h"
  #include "linux/LinuxEvent.h"
  #include "linux/LinuxMutex.h"
  #include "linux/LinuxSemaphore.h"
  #include "linux/LinuxPipe.h"
  #include "linux/LinuxTimer.h"
  #include "linux/LinuxSharedMemory.h"

#else
  #error "Platform not detected"
#endif

#endif
