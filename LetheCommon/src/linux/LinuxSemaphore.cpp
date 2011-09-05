#include "linux/LinuxSemaphore.h"
#include "LetheInternal.h"
#include "LetheTypes.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include "eventfd-lethe.h"
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace lethe;

const std::string LinuxSemaphore::s_eventfdDevice("/dev/eventfd-lethe");

LinuxSemaphore::LinuxSemaphore(uint32_t maxCount,
                               uint32_t initialCount) :
  WaitObject(open(s_eventfdDevice.c_str(), O_RDWR))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("eventfd open", lastError());

  if(!setCloseOnExec(getHandle()))
  {
    close(getHandle());
    throw std::bad_syscall("fcntl", lastError());
  }

  if(ioctl(getHandle(), EFD_SET_SEMAPHORE_MODE, initialCount) != 0)
  {
    close(getHandle());
    throw std::bad_syscall("eventfd ioctl EFD_SET_SEMAPHORE_MODE", lastError());
  }

  if(ioctl(getHandle(), EFD_SET_MAX_VALUE, maxCount) != 0)
  {
    close(getHandle());
    throw std::bad_syscall("eventfd ioctl EFD_SET_MAX_VALUE", lastError());
  }

  if(ioctl(getHandle(), EFD_SET_WAITREAD_MODE, true) != 0)
  {
    close(getHandle());
    throw std::bad_syscall("eventfd ioctl EFD_SET_WAITREAD_MODE", lastError());
  }
}

LinuxSemaphore::LinuxSemaphore(Handle handle) :
  WaitObject(handle)
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::invalid_argument("handle");

  // Make sure the handle is for an eventfd-lethe object
  struct stat handleInfo;
  if(fstat(handle, &handleInfo) != 0 || handleInfo.st_dev != EVENTFD_LETHE_MAJOR)
  {
    close(handle);
    throw std::bad_syscall("eventfd fstat", lastError());
  }

  if(ioctl(getHandle(), EFD_GET_MODE) != EFD_SEMAPHORE_MODE)
  {
    close(getHandle());
    throw std::runtime_error("eventfd ioctl in unexpected mode");
  }

  if(!setCloseOnExec(getHandle()))
  {
    close(getHandle());
    throw std::bad_syscall("fcntl", lastError());
  }
}

LinuxSemaphore::~LinuxSemaphore()
{
  close(getHandle());
}

void LinuxSemaphore::lock(uint32_t timeout)
{
  if(WaitForObject(getHandle(), timeout) != WaitSuccess)
    throw std::runtime_error("failed to wait for semaphore");
}

void LinuxSemaphore::unlock(uint32_t count)
{
  uint64_t internalCount(count);

  if(write(getHandle(), &internalCount, sizeof(internalCount)) != sizeof(internalCount))
    throw std::bad_syscall("eventfd write", lastError());
}

void LinuxSemaphore::error()
{
  if(ioctl(getHandle(), EFD_SET_ERROR, true) != 0)
    throw std::bad_syscall("eventfd ioctl EFD_SET_ERROR", lastError());
}

