#include "windows/WindowsEvent.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <Windows.h>

WindowsEvent::WindowsEvent(bool initialState, bool autoReset) :
  WaitObject(CreateEvent(NULL, !autoReset, initialState, NULL))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create event: " + lastError());
}

WindowsEvent::~WindowsEvent()
{
  CloseHandle(getHandle());
}

void WindowsEvent::set()
{
  if(!SetEvent(getHandle()))
    throw Exception("Failed to set event: " + lastError());
}

void WindowsEvent::reset()
{
  if(!ResetEvent(getHandle()))
    throw Exception("Failed to reset event: " + lastError());
}
