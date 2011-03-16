#include "linux/LinuxEvent.h"
#include "LetheTypes.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <unistd.h>
#include <fcntl.h>
#include "eventfd.h"
#include <errno.h>

using namespace lethe;

LinuxEvent::LinuxEvent(bool initialState, bool autoReset) :
  WaitObject(eventfd(initialState,
                     EFD_NONBLOCK | (autoReset ? EFD_WAITREAD : 0)))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("eventfd", lastError());
}

LinuxEvent::LinuxEvent(Handle handle) :
  WaitObject(handle)
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::invalid_argument("handle");

  struct stat handleInfo;
  if(fstat(handle, &handleInfo) != 0) // TODO: check if handle is for an eventfd
  {
    close(handle);
    throw std::bad_syscall("fstat", lastError());
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
    throw std::bad_syscall("write to eventfd", lastError());
}

void LinuxEvent::reset()
{
  uint64_t state;

  if(read(getHandle(), &state, sizeof(state)) != sizeof(state) && errno != EAGAIN)
    throw std::bad_syscall("read from eventfd", lastError());
}
