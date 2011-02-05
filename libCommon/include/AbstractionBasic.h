#ifndef _ABSTRACTIONBASIC_H
#define _ABSTRACTIONBASIC_H

/*
 * The AbstractionBasic.h header includes and typedefs basic objects for
 *  use by users or the more complex objects in AbstractionComplex.h or
 *  the functions in AbstractionFunctions.h.
 */

#include "WaitObject.h"

#if defined(__WIN32__) || defined(_WIN32)

  class WindowsWaitSet;
  class WindowsEvent;
  class WindowsMutex;
  class WindowsSemaphore;
  class WindowsPipe;
  class WindowsTimer;

  typedef WindowsWaitSet WaitSet;
  typedef WindowsEvent Event;
  typedef WindowsMutex Mutex;
  typedef WindowsSemaphore Semaphore;
  typedef WindowsPipe Pipe;
  typedef WindowsTimer Timer;

  #include "windows/WindowsWaitSet.h"
  #include "windows/WindowsEvent.h"
  #include "windows/WindowsMutex.h"
  #include "windows/WindowsSemaphore.h"
  #include "windows/WindowsPipe.h"
  #include "windows/WindowsTimer.h"

#elif defined(__linux__)

  class LinuxWaitSet;
  class LinuxEvent;
  class LinuxMutex;
  class LinuxSemaphore;
  class LinuxPipe;
  class LinuxTimer;

  typedef LinuxWaitSet WaitSet;
  typedef LinuxEvent Event;
  typedef LinuxMutex Mutex;
  typedef LinuxSemaphore Semaphore;
  typedef LinuxPipe Pipe;
  typedef LinuxTimer Timer;

  #include "linux/LinuxWaitSet.h"
  #include "linux/LinuxEvent.h"
  #include "linux/LinuxMutex.h"
  #include "linux/LinuxSemaphore.h"
  #include "linux/LinuxPipe.h"
  #include "linux/LinuxTimer.h"

#else
  #error "Platform not detected"
#endif

#endif
