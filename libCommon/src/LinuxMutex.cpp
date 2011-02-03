#include "linux/LinuxMutex.h"
#include "Exception.h"
#include <poll.h>

LinuxMutex::LinuxMutex(bool locked) :
  LinuxSemaphore(1, (locked ? 0 : 1))
{
  // Do nothing
}

LinuxMutex::~LinuxMutex()
{
  // Do nothing
}

void LinuxMutex::lock(uint32_t timeout)
{
  LinuxSemaphore::lock(timeout);
}

void LinuxMutex::unlock()
{
  LinuxSemaphore::unlock(1);
}

int LinuxMutex::getHandle()
{
  return LinuxSemaphore::getHandle();
}

