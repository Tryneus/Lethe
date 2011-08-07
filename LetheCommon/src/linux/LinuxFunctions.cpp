#include "LetheFunctions.h"
#include "LetheBasic.h"
#include "LetheException.h"
#include "LetheInternal.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <poll.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctime>

void lethe::sleep_ms(uint32_t timeout)
{
  struct timespec remaining;

  remaining.tv_sec = timeout / 1000;
  remaining.tv_nsec = (timeout % 1000) * 1000000;

  // Loop in case of EINTR - nanosleep should update remainder
  // TODO: make sure there isn't a time leak here - update using the end time instead?
  while(nanosleep(&remaining, &remaining) == -1 && errno == EINTR);
}

std::string lethe::getErrorString(uint32_t errorCode)
{
  char buffer[200];
  return std::string(strerror_r(errorCode, buffer, 200));
}

std::string lethe::lastError()
{
  return lethe::getErrorString(errno);
}

uint32_t lethe::seedRandom(uint32_t seed)
{
  if(seed == 0)
  {
    timeval currentTime;
    gettimeofday(&currentTime, NULL);

    seed = currentTime.tv_usec | (currentTime.tv_sec << 20);
  }

  srand(seed);

  return seed;
}

uint64_t lethe::getTime()
{
  timeval currentTime;

  if(gettimeofday(&currentTime, NULL) != 0)
    throw std::bad_syscall("gettimeofday", lethe::lastError());

  return (currentTime.tv_sec * 1000) + (currentTime.tv_usec / 1000);
}

std::string lethe::getTimeString()
{
  std::ostringstream stream;
  char buffer[100];
  struct tm* currentTime;
  struct timeval rawTime;

  gettimeofday(&rawTime, NULL);
  currentTime = localtime(&rawTime.tv_sec);
  strftime(buffer, sizeof(buffer), "%b %d %H:%M:%S.", currentTime);

  stream << buffer << std::setfill('0') << std::setw(3) << (rawTime.tv_usec / 1000);

  return stream.str();
}

uint32_t lethe::getProcessId()
{
  return static_cast<uint32_t>(getpid());
}

uint32_t lethe::getThreadId()
{
  return static_cast<uint32_t>(pthread_self());
}

lethe::WaitResult lethe::WaitForObject(lethe::WaitObject& obj, uint32_t timeout)
{
  lethe::WaitResult result = lethe::WaitSuccess;

  if(!obj.preWaitCallback())
  {
    try
    {
      result = lethe::WaitForObject(obj.getHandle(), timeout);
    }
    catch(...)
    {
      obj.postWaitCallback(lethe::WaitError);
      throw;
    }
  }

  obj.postWaitCallback(result);
  return result;
}

lethe::WaitResult lethe::WaitForObject(lethe::Handle handle, uint32_t timeout)
{
  uint32_t endTime = lethe::getEndTime(timeout);
  lethe::WaitResult result = lethe::WaitSuccess;
  struct pollfd pollData;
  int pollResult;

  while(true)
  {
    pollData.fd = handle;
    pollData.events = POLLIN | POLLERR | POLLHUP;

    pollResult = poll(&pollData, 1, timeout);

    if(pollResult == 1)
    {
      if(pollData.revents & (POLLERR | POLLNVAL | POLLHUP))
        result = lethe::WaitAbandoned;
      break;
    }
    else if(pollResult == 0)
    {
      result = lethe::WaitTimeout;
      break;
    }
    else if(errno == EINTR)
    {
      timeout = lethe::getTimeout(endTime);
      continue;
    }
    else if(errno == EBADF) // TODO: is this the right errno? maybe others too?
    {
      result = lethe::WaitAbandoned;
      break;
    }
    else
      throw std::bad_syscall("poll", lethe::lastError());
  }

  return result;
}

