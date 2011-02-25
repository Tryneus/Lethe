#include "linux/LinuxTimer.h"
#include "LetheTypes.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <sys/timerfd.h>
#include <fcntl.h>
#include <errno.h>

using namespace lethe;

LinuxTimer::LinuxTimer() :
  WaitObject(timerfd_create(CLOCK_MONOTONIC, O_NONBLOCK))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("timerfd_create", lastError());
}

LinuxTimer::~LinuxTimer()
{
  close(getHandle());
}

void LinuxTimer::start(uint32_t timeout)
{
  itimerspec elapseTime;

  elapseTime.it_value.tv_sec = timeout / 1000;
  elapseTime.it_value.tv_nsec = (timeout % 1000) * 1000000;
  elapseTime.it_interval.tv_sec = 0;
  elapseTime.it_interval.tv_nsec = 0;


  if(timerfd_settime(getHandle(), 0, &elapseTime, NULL) != 0)
    throw std::bad_syscall("timerfd_settime", lastError());
}

void LinuxTimer::clear()
{
  itimerspec elapseTime;

  elapseTime.it_value.tv_sec = 0;
  elapseTime.it_value.tv_nsec = 0;
  elapseTime.it_interval.tv_sec = 0;
  elapseTime.it_interval.tv_nsec = 0;

  if(timerfd_settime(getHandle(), 0, &elapseTime, NULL) != 0)
    throw std::bad_syscall("timerfd_settime", lastError());
}
