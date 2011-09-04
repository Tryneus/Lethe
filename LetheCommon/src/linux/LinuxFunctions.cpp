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
#include <iostream>

uint32_t lethe::getParentProcessId()
{
  return getppid();
}

uint32_t lethe::createProcess(const std::string& command,
                              const std::vector<std::string>& arguments,
                              const std::vector<std::pair<std::string, std::string> >& environment)
{
  uint32_t pid = fork();

  if(pid == 0)
  {
    std::vector<std::string> envConverted;
    const char* argStrings[arguments.size() + 2];
    const char* envStrings[environment.size() + 1];

    // Convert environment into the correct format
    for(size_t i = 0; i < environment.size(); ++i)
    {
      envConverted[i] = environment[i].first + "=" + environment[i].second;
      envStrings[i] = envConverted[i].c_str();
    }

    envStrings[environment.size()] = NULL;

    // The first argument should be the command being run
    argStrings[0] = command.c_str();

    for(size_t i = 0; i < arguments.size(); ++i)
      argStrings[i + 1] = arguments[i].c_str();

    argStrings[arguments.size() + 1] = NULL;

    execve(command.c_str(), const_cast<char* const*>(argStrings), const_cast<char* const*>(envStrings));

    // Exec should not return, if we get to this line, an error has occurred
    throw std::bad_syscall("execve", lastError());
  }

  return pid;
}

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
  uint32_t endTime = lethe::getEndTime(timeout);
  lethe::WaitResult result = lethe::WaitSuccess;
  struct pollfd pollData;
  int pollResult;

  while(true)
  {
    pollData.fd = obj.getHandle();
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
    else if(errno == EBADF)
    {
      result = lethe::WaitAbandoned;
      break;
    }
    else
      throw std::bad_syscall("poll", lethe::lastError());
  }

  return result;
}

