#include "linux/LinuxTimer.h"
#include "AbstractionTypes.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <sys/timerfd.h>
#include <fcntl.h>
#include <errno.h>

LinuxTimer::LinuxTimer() :
  WaitObject(timerfd_create(CLOCK_MONOTONIC, O_NONBLOCK))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create timer: " + lastError());
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
    throw Exception("Failed to start timer: " + lastError());
}

void LinuxTimer::stop()
{
  itimerspec elapseTime;

  elapseTime.it_value.tv_sec = 0;
  elapseTime.it_value.tv_nsec = 0;
  elapseTime.it_interval.tv_sec = 0;
  elapseTime.it_interval.tv_nsec = 0;

  if(timerfd_settime(getHandle(), 0, &elapseTime, NULL) != 0)
    throw Exception("Failed to stop timer: " + lastError());
}

void LinuxTimer::clear()
{
  stop();

  uint64_t buffer;
  if(read(getHandle(), &buffer, sizeof(buffer)) != sizeof(buffer) && errno != EAGAIN)
    throw Exception("Failed to clear timer: " + lastError());
}
