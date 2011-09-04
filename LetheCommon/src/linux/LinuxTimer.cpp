#include "timerfd-lethe.h"
#include "linux/LinuxTimer.h"
#include "LetheInternal.h"
#include "LetheTypes.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

using namespace lethe;

const std::string LinuxTimer::s_timerfdDevice("/dev/timerfd-lethe");

LinuxTimer::LinuxTimer(uint32_t timeout, bool periodic, bool autoReset) :
  WaitObject(open(s_timerfdDevice.c_str(), O_RDWR))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("timerfd open", lastError());

  if(!setCloseOnExec(getHandle()))
  {
    close(getHandle());
    throw std::bad_syscall("fcntl", lastError());
  }

  if(ioctl(getHandle(), TFD_SET_WAITREAD_MODE, autoReset) != 0)
  {
    close(getHandle());
    throw std::bad_syscall("timerfd ioctl TFD_SET_WAITREAD_MODE", lastError());
  }

  start(timeout, periodic);
}

LinuxTimer::LinuxTimer(Handle handle) :
  WaitObject(handle)
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::invalid_argument("handle");

  struct stat handleInfo;
  if(fstat(handle, &handleInfo) != 0 || handleInfo.st_dev != TIMERFD_LETHE_MAJOR)
  {
    close(handle);
    throw std::bad_syscall("timerfd fstat", lastError());
  }

  if(!setCloseOnExec(getHandle()))
  {
    close(getHandle());
    throw std::bad_syscall("fcntl", lastError());
  }
}

LinuxTimer::~LinuxTimer()
{
  close(getHandle());
}

void LinuxTimer::start(uint32_t timeout, bool periodic)
{
  timespec elapseTime;

  elapseTime.tv_sec = timeout / 1000;
  elapseTime.tv_nsec = (timeout % 1000) * 1000000;

  if(periodic)
  {
    if(ioctl(getHandle(), TFD_SET_PERIODIC_TIME, &elapseTime) != 0)
      throw std::bad_syscall("timerfd ioctl TFD_SET_PERIODIC_TIME", lastError());
  }
  else
  {
    if(ioctl(getHandle(), TFD_SET_RELATIVE_TIME, &elapseTime) != 0)
      throw std::bad_syscall("timerfd ioctl TFD_SET_RELATIVE_TIME", lastError());
  }
}

void LinuxTimer::clear()
{
  timespec elapseTime;

  elapseTime.tv_sec = 0;
  elapseTime.tv_nsec = 0;

  if(ioctl(getHandle(), TFD_SET_RELATIVE_TIME, &elapseTime) != 0)
    throw std::bad_syscall("timerfd ioctl TFD_SET_RELATIVE_TIME", lastError());
}

void LinuxTimer::error()
{
  if(ioctl(getHandle(), TFD_SET_ERROR, true) != 0)
    throw std::bad_syscall("timerfd ioctl TFD_SET_ERROR", lastError());
}
