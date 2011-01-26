#include "WindowsTimer.h"
#include "Exception.h"
#include "Abstraction.h"

// TODO: This is a bit of a hack to clear a timer
//  we just reset it with a really long timeout
const int64_t WindowsTimer::s_resetTimeout(-315360000000000);

WindowsTimer::WindowsTimer() :
  m_handle(CreateWaitableTimer(NULL, true, NULL))
{
  if(m_handle == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create timer: " + lastError());
}

WindowsTimer::~WindowsTimer()
{
  if(!CloseHandle(m_handle))
    throw Exception("Failed to close timer: " + lastError());
}

HANDLE WindowsTimer::getHandle() const
{
  return m_handle;
}

void WindowsTimer::start(uint32_t timeout)
{
  LARGE_INTEGER elapseTime;
  elapseTime.QuadPart = -(static_cast<int64_t>(timeout) * 10000);

  if(!SetWaitableTimer(m_handle, &elapseTime, 0, NULL, NULL, false))
    throw Exception("Failed to start timer: " + lastError());
}

void WindowsTimer::stop()
{
  if(!CancelWaitableTimer(m_handle))
    throw Exception("Failed to stop timer: " + lastError());
}

void WindowsTimer::clear()
{
  LARGE_INTEGER elapseTime;
  elapseTime.QuadPart = s_resetTimeout;

  if(!SetWaitableTimer(m_handle, &elapseTime, 0, NULL, NULL, false))
    throw Exception("Failed to clear timer: " + lastError());
}
