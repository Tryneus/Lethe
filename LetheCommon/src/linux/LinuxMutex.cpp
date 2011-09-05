#include "linux/LinuxMutex.h"
#include "LetheInternal.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include "eventfd-lethe.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

using namespace lethe;

const std::string LinuxMutex::s_eventfdDevice("/dev/eventfd-lethe");

LinuxMutex::LinuxMutex(bool locked) :
  WaitObject(open(s_eventfdDevice.c_str(), O_RDWR))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("eventfd open", lastError());

  if(!setCloseOnExec(getHandle()))
  {
    close(getHandle());
    throw std::bad_syscall("fcntl", lastError());
  }

  if(ioctl(getHandle(), EFD_SET_MUTEX_MODE, locked) != 0)
  {
    close(getHandle());
    throw std::bad_syscall("eventfd ioctl EFD_SET_MUTEX_MODE", lastError());
  }

  if(ioctl(getHandle(), EFD_SET_WAITREAD_MODE, true) != 0)
  {
    close(getHandle());
    throw std::bad_syscall("eventfd ioctl EFD_SET_WAITREAD_MODE", lastError());
  }
}

LinuxMutex::LinuxMutex(Handle handle) :
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

  if(ioctl(getHandle(), EFD_GET_MODE) != EFD_MUTEX_MODE)
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

LinuxMutex::~LinuxMutex()
{
  close(getHandle());
}

void LinuxMutex::lock(uint32_t timeout)
{
  if(WaitForObject(getHandle(), timeout) != WaitSuccess)
    throw std::runtime_error("failed to wait for mutex");
}

void LinuxMutex::unlock()
{
  uint64_t buffer(1);
  if(write(getHandle(), &buffer, sizeof(buffer)) != sizeof(buffer))
    throw std::bad_syscall("eventfd write", lastError());
}

void LinuxMutex::error()
{
  if(ioctl(getHandle(), EFD_SET_ERROR, true) != 0)
    throw std::bad_syscall("eventfd ioctl EFD_SET_ERROR", lastError());
}

