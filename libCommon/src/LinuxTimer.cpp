#include "LinuxTimer.h"
#include "Exception.h"
#include <sys/timerfd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>

LinuxTimer::LinuxTimer() :
  m_fd(timerfd_create(CLOCK_MONOTONIC, O_NONBLOCK))
{
  if(m_fd == -1)
    throw Exception("Failed to create timer");
}

LinuxTimer::~LinuxTimer()
{
  close(m_fd);
}
  
int LinuxTimer::getHandle() const
{
  return m_fd;
}
  
void LinuxTimer::start(uint32_t timeout)
{
  itimerspec elapseTime;
  
  elapseTime.it_interval.tv_sec = timeout / 1000;
  elapseTime.it_interval.tv_nsec = timeout * 1000000;
  elapseTime.it_value.tv_sec = 0;
  elapseTime.it_value.tv_nsec = 0;
  
  if(timerfd_settime(m_fd, 0, &elapseTime, NULL) == -1)
    throw Exception("Failed to start timer");
}

void LinuxTimer::stop()
{
  itimerspec elapseTime;
  memset(&elapseTime, 0, sizeof(elapseTime));

  if(timerfd_settime(m_fd, 0, &elapseTime, NULL) == -1)
    throw Exception("Failed to stop timer");
}

void LinuxTimer::clear()
{
  stop();
  
  uint64_t buffer;
  if(read(m_fd, &buffer, sizeof(buffer)) != sizeof(buffer))
    throw Exception("Failed to read from timer fd");
}
