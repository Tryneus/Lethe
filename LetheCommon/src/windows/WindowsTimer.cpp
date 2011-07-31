#define __STDC_LIMIT_MACROS // Get INT64_MIN from stdint.h
#include "windows/WindowsTimer.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <Windows.h>
#include <sstream>

using namespace lethe;

const int64_t WindowsTimer::s_resetTimeout(INT64_MIN);
const std::string WindowsTimer::s_timerBaseName("Global\\lethe-timer-");
WindowsAtomic WindowsTimer::s_uniqueId(0);

WindowsTimer::WindowsTimer(uint32_t timeout) :
  WaitObject(NULL)
{
  std::stringstream str;
  str << s_timerBaseName << getProcessId() << "-" << s_uniqueId.increment();
  m_name.assign(str.str());

  setWaitHandle(CreateWaitableTimer(NULL, true, m_name.c_str()));

  if(getHandle() == NULL)
    throw std::bad_syscall("CreateWaitableTimer", lastError());

  if(timeout != INFINITE)
    start(timeout);
}

WindowsTimer::WindowsTimer(const std::string& name) :
  WaitObject(NULL),
  m_name(name)
{
  setWaitHandle(OpenWaitableTimer(TIMER_MODIFY_STATE |
                                  TIMER_QUERY_STATE |
                                  SYNCHRONIZE, false, m_name.c_str()));

  if(getHandle() == NULL)
    throw std::bad_syscall("CreateWaitableTimer", lastError());
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
    throw std::bad_syscall("SetWaitableTimer", lastError());
}

void WindowsTimer::clear()
{
  LARGE_INTEGER elapseTime;
  elapseTime.QuadPart = s_resetTimeout;

  if(!SetWaitableTimer(getHandle(), &elapseTime, 0, NULL, NULL, false))
    throw std::bad_syscall("SetWaitableTimer", lastError());
}
