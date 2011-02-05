#include "linux/LinuxEvent.h"
#include "AbstractionTypes.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <unistd.h>
#include <fcntl.h>
#include "eventfd.h"
#include <errno.h>

LinuxEvent::LinuxEvent(bool initialState, bool autoReset) :
  WaitObject(eventfd(initialState,
                     EFD_NONBLOCK | (autoReset ? EFD_WAITREAD : 0)))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create event: " + lastError());
}

LinuxEvent::~LinuxEvent()
{
  close(getHandle());
}

void LinuxEvent::set()
{
  uint64_t state(1);

  if(write(getHandle(), &state, sizeof(state)) != sizeof(state) && errno != EAGAIN)
    throw Exception("Failed to set event: " + lastError());
}

void LinuxEvent::reset()
{
  uint64_t state;

  if(read(getHandle(), &state, sizeof(state)) != sizeof(state) && errno != EAGAIN)
    throw Exception("Failed to reset event: " + lastError());
}
