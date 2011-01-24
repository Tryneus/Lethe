#ifndef _ABSTRACTION_H
#define _ABSTRACTION_H

#include <string>
#include <vector>

#if defined(__WIN32__) || defined(_WIN32)
  #include <new.h>
  #include "stdint.h"
  #include "Windows.h"

  // Predeclarations in case of circular include
  class WindowsHandleSet;
  class WindowsEvent;
  class WindowsMutex;
  class WindowsSemaphore;
  class WindowsPipe;
  class WindowsThread;
  class WindowsTimer;

  #include "WindowsHandleSet.h"
  #include "WindowsEvent.h"
  #include "WindowsMutex.h"
  #include "WindowsSemaphore.h"
  #include "WindowsPipe.h"
  #include "WindowsThread.h"
  #include "WindowsTimer.h"

  typedef WindowsHandleSet HandleSet;
  typedef HANDLE Handle;
  typedef WindowsEvent Event;
  typedef WindowsMutex Mutex;
  typedef WindowsSemaphore Semaphore;
  typedef WindowsPipe Pipe;
  typedef WindowsThread Thread;
  typedef WindowsTimer Timer;

#elif defined(__linux__)
  #include <stdint.h>
  #include <stddef.h>
  #include <stdlib.h>

  // Predeclarations in case of circular include
  class LinuxHandleSet;
  class LinuxEvent;
  class LinuxMutex;
  class LinuxSemaphore;
  class LinuxPipe;
  class LinuxThread;
  class LinuxTimer;

  #include "LinuxHandleSet.h"
  #include "LinuxEvent.h"
  #include "LinuxMutex.h"
  #include "LinuxSemaphore.h"
  #include "LinuxPipe.h"
  #include "LinuxThread.h"
  #include "LinuxTimer.h"

  typedef LinuxHandleSet HandleSet;
  typedef int Handle;
  typedef LinuxEvent Event;
  typedef LinuxMutex Mutex;
  typedef LinuxSemaphore Semaphore;
  typedef LinuxPipe Pipe;
  typedef LinuxThread Thread;
  typedef LinuxTimer Timer;

  void Sleep(uint32_t timeout);

#else
  #error Platform not detected
#endif

void getFileList(const std::string& directory,
                 std::vector<std::string>& fileList);

std::string getTimeString();

uint32_t seedRandom(uint32_t seed = 0);

int WaitForObject(Handle handle, uint32_t timeout);

std::string lastError();

#endif
