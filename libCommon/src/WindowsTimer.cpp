#define __STDC_LIMIT_MACROS // Get INT64_MIN from stdint.h
#include "windows/WindowsTimer.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <Windows.h>

const int64_t WindowsTimer::s_resetTimeout(INT64_MIN);

WindowsTimer::WindowsTimer() :
  WaitObject(CreateWaitableTimer(NULL, true, NULL))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create timer: " + lastError());
}

WindowsTimer::~WindowsTimer()
{
  CloseHandle(getHandle());
}

void WindowsTimer::start(uint32_t timeout)
{
  LARGE_INTEGER elapseTime;
  elapseTime.QuadPart = -(static_cast<int64_t>(timeout) * 10000);

  if(!SetWaitableTimer(getHandle(), &elapseTime, 0, NULL, NULL, false))
    throw Exception("Failed to start timer: " + lastError());
}

void WindowsTimer::clear()
{
  LARGE_INTEGER elapseTime;
  elapseTime.QuadPart = s_resetTimeout;

  if(!SetWaitableTimer(getHandle(), &elapseTime, 0, NULL, NULL, false))
    throw Exception("Failed to clear timer: " + lastError());
}
