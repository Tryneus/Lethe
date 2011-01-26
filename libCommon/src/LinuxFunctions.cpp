#include "LinuxHandleSet.h"
#include "Exception.h"
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

void Sleep(uint32_t timeout)
{
  usleep(timeout * 1000);
}

std::string lastError()
{
  int errorCode = errno;
  char buffer[200];

  return std::string(strerror_r(errorCode, buffer, 200));
}

uint32_t seedRandom(uint32_t seed)
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

void getFileList(const std::string& directory,
                 std::vector<std::string>& fileList)
{
  throw Exception("LinuxFunctions::getFileList not yet implemented");
  directory.length();
  fileList.size();
}

std::string getTimeString()
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

int WaitForObject(int fd, uint32_t timeout)
{
  struct pollfd object;

  object.fd = fd;
  object.events = POLLIN;
  
  switch(poll(&object, 1, timeout))
  {
  case 1:
    if(object.revents & (POLLERR | POLLNVAL | POLLHUP))
      return WaitAbandoned;

    return WaitSuccess;

  case 0:
    return WaitTimeout;

  default:
    throw Exception("Failed to wait: " + lastError());
  }
}

