#include "windows/WindowsTimer.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <Windows.h>

// TODO: This is a bit of a hack to clear a timer
//  we just reset it with a really long timeout
const int64_t WindowsTimer::s_resetTimeout(-315360000000000);

WindowsTimer::WindowsTimer() :
  WaitObject(CreateWaitableTimer(NULL, true, NULL))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create timer: " + lastError());
}

WindowsTimer::~WindowsTimer()
{
  if(!CloseHandle(getHandle()))
    throw Exception("Failed to close timer: " + lastError());
}

void WindowsTimer::start(uint32_t timeout)
{
  LARGE_INTEGER elapseTime;
  elapseTime.QuadPart = -(static_cast<int64_t>(timeout) * 10000);

  if(!SetWaitableTimer(getHandle(), &elapseTime, 0, NULL, NULL, false))
    throw Exception("Failed to start timer: " + lastError());
}

void WindowsTimer::stop()
{
  if(!CancelWaitableTimer(getHandle()))
    throw Exception("Failed to stop timer: " + lastError());
}

void WindowsTimer::clear()
{
  LARGE_INTEGER elapseTime;
  elapseTime.QuadPart = s_resetTimeout;

  if(!SetWaitableTimer(getHandle(), &elapseTime, 0, NULL, NULL, false))
    throw Exception("Failed to clear timer: " + lastError());
}
