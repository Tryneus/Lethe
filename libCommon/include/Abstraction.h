#ifndef _ABSTRACTION_H
#define _ABSTRACTION_H

#include <string>
#include <vector>

#if defined(__WIN32__) || defined(_WIN32)
  #include <new.h>
  #include "stdint.h"
  #include "Windows.h"

  class WindowsHandleSet; // Predeclaration in case of circular include
  #include "WindowsHandleSet.h"
  typedef WindowsHandleSet HandleSet;
  typedef HANDLE Handle;

  class WindowsEvent; // Predeclaration in case of circular include
  #include "WindowsEvent.h"
  typedef WindowsEvent Event;

  class WindowsMutex; // Predeclaration in case of circular include
  #include "WindowsMutex.h"
  typedef WindowsMutex Mutex;

  class WindowsThread; // Predeclaration in case of circular include
  #include "WindowsThread.h"
  typedef WindowsThread Thread;
  
  class WindowsTimer; // Predeclaration in case of circular include
  #include "WindowsTimer.h"
  typedef WindowsTimer Timer;
  
#elif defined(__linux__)
  #include <stdint.h>
  #include <stddef.h>
  #include <stdlib.h>

  class LinuxHandleSet;
  #include "LinuxHandleSet.h"
  typedef LinuxHandleSet HandleSet;
  typedef int Handle;

  class LinuxEvent;
  #include "LinuxEvent.h"
  typedef LinuxEvent Event;

  class LinuxMutex; // Predeclaration in case of circular include
  #include "LinuxMutex.h"
  typedef LinuxMutex Mutex;
  
  class LinuxThread; // Predeclaration in case of circular include
  #include "LinuxThread.h"
  typedef LinuxThread Thread;
  
  class LinuxTimer; // Predeclaration in case of circular include
  #include "LinuxTimer.h"
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

#endif
