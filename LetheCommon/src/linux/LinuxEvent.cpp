#include "linux/LinuxEvent.h"
#include "LetheInternal.h"
#include "LetheTypes.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "eventfd-lethe.h"
#include <errno.h>

using namespace lethe;

const std::string LinuxEvent::s_eventfdDevice("/dev/eventfd-lethe");

LinuxEvent::LinuxEvent(bool initialState, bool autoReset) :
  WaitObject(open(s_eventfdDevice.c_str(), O_RDWR))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("eventfd open", lastError());

  if(!setCloseOnExec(getHandle()))
  {
    close(getHandle());
    throw std::bad_syscall("fcntl", lastError());
  }

  if(ioctl(getHandle(), EFD_SET_EVENT_MODE, initialState) != 0)
  {
    close(getHandle());
    throw std::bad_syscall("eventfd ioctl EFD_SET_EVENT_MODE", lastError());
  }

  if(ioctl(getHandle(), EFD_SET_WAITREAD_MODE, autoReset) != 0)
  {
    close(getHandle());
    throw std::bad_syscall("eventfd ioctl EFD_SET_WAITREAD_MODE", lastError());
  }
}

LinuxEvent::LinuxEvent(Handle handle) :
  WaitObject(handle)
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::invalid_argument("handle");

  // Make sure the handle is for an eventfd-lethe object
  // TODO: Check the mode of the file descriptor
  struct stat handleInfo;
  if(fstat(handle, &handleInfo) != 0 || handleInfo.st_dev != EVENTFD_LETHE_MAJOR)
  {
    close(handle);
    throw std::bad_syscall("eventfd fstat", lastError());
  }

  if(!setCloseOnExec(getHandle()))
  {
    close(getHandle());
    throw std::bad_syscall("fcntl", lastError());
  }
}

LinuxEvent::~LinuxEvent()
{
  close(getHandle());
}

void LinuxEvent::set()
{
  uint64_t state(1);

  if(write(getHandle(), &state, sizeof(state)) != sizeof(state))
    throw std::bad_syscall("eventfd write", lastError());
}

void LinuxEvent::reset()
{
  uint64_t state;

  if(read(getHandle(), &state, sizeof(state)) != sizeof(state) && errno != EAGAIN)
    throw std::bad_syscall("eventfd read", lastError());
}

void LinuxEvent::error()
{
  if(ioctl(getHandle(), EFD_SET_ERROR, true) != 0)
    throw std::bad_syscall("eventfd ioctl EFD_SET_ERROR", lastError());
}

