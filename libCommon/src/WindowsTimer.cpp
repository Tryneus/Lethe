#include "WindowsTimer.h"
#include "Exception.h"

// TODO: This is a bit of a hack, to clear a timer, we just reset it with a really long timeout
const int64_t WindowsTimer::s_resetTimeout(-315360000000000);

WindowsTimer::WindowsTimer() :
  m_handle(CreateWaitableTimer(NULL, true, NULL))
{
  if(m_handle == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create timer");
}

WindowsTimer::~WindowsTimer()
{
  CloseHandle(m_handle);
}

HANDLE WindowsTimer::getHandle() const
{
  return m_handle;
}

void WindowsTimer::start(uint32_t timeout)
{
  LARGE_INTEGER elapseTime;
  elapseTime.QuadPart = -(static_cast<int64_t>(timeout) * 10000);

  SetWaitableTimer(m_handle, &elapseTime, 0, NULL, NULL, false);
}

void WindowsTimer::stop()
{
  CancelWaitableTimer(m_handle);
}

void WindowsTimer::clear()
{
  LARGE_INTEGER elapseTime;
  elapseTime.QuadPart = s_resetTimeout;

  SetWaitableTimer(m_handle, &elapseTime, 0, NULL, NULL, false);
}