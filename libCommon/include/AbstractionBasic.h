#ifndef _ABSTRACTIONBASIC_H
#define _ABSTRACTIONBASIC_H

/*
 * The AbstractionBasic.h header includes and typedefs basic objects for
 *  use by users or the more complex objects in AbstractionComplex.h or
 *  the functions in AbstractionFunctions.h.
 */

#if defined(__WIN32__) || defined(_WIN32)
  #if defined(_MSC_VER)
    #include <new.h> // Include placement new for Visual C++
    #include "stdint.h" // Include local stdint.h when using Visual C++
  #else
    #include <stdint.h>
  #endif

  #include <Windows.h>

  class WindowsHandleSet;
  class WindowsEvent;
  class WindowsMutex;
  class WindowsSemaphore;
  class WindowsPipe;
  class WindowsTimer;

  typedef WindowsHandleSet HandleSet;
  typedef HANDLE Handle;
  typedef WindowsEvent Event;
  typedef WindowsMutex Mutex;
  typedef WindowsSemaphore Semaphore;
  typedef WindowsPipe Pipe;
  typedef WindowsTimer Timer;

  #include "windows/WindowsHandleSet.h"
  #include "windows/WindowsEvent.h"
  #include "windows/WindowsMutex.h"
  #include "windows/WindowsSemaphore.h"
  #include "windows/WindowsPipe.h"
  #include "windows/WindowsTimer.h"

#elif defined(__linux__)
  #include <stdint.h>

  class LinuxHandleSet;
  class LinuxEvent;
  class LinuxMutex;
  class LinuxSemaphore;
  class LinuxPipe;
  class LinuxTimer;

  typedef LinuxHandleSet HandleSet;
  typedef int Handle;
  typedef LinuxEvent Event;
  typedef LinuxMutex Mutex;
  typedef LinuxSemaphore Semaphore;
  typedef LinuxPipe Pipe;
  typedef LinuxTimer Timer;

  #include "linux/LinuxHandleSet.h"
  #include "linux/LinuxEvent.h"
  #include "linux/LinuxMutex.h"
  #include "linux/LinuxSemaphore.h"
  #include "linux/LinuxPipe.h"
  #include "linux/LinuxTimer.h"

#else
  #error Platform not detected
#endif

#endif
