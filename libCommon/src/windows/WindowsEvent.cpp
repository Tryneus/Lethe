#include "windows/WindowsEvent.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include <Windows.h>

WindowsEvent::WindowsEvent(bool initialState, bool autoReset) :
  WaitObject(CreateEvent(NULL, !autoReset, initialState, NULL))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("CreateEvent", lastError());
}

WindowsEvent::~WindowsEvent()
{
  CloseHandle(getHandle());
}

void WindowsEvent::set()
{
  if(!SetEvent(getHandle()))
    throw std::bad_syscall("SetEvent", lastError());
}

void WindowsEvent::reset()
{
  if(!ResetEvent(getHandle()))
    throw std::bad_syscall("ResetEVent", lastError());
}
