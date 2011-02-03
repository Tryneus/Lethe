#include "linux/LinuxEvent.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <unistd.h>
#include <fcntl.h>
#include "eventfd.h"
#include <errno.h>

LinuxEvent::LinuxEvent(bool initialState, bool autoReset) :
  m_event(eventfd(initialState,
                  EFD_NONBLOCK | (autoReset ? EFD_WAITREAD : 0)))
{
  if(m_event == -1)
    throw Exception("Failed to create event: " + lastError());
}

LinuxEvent::~LinuxEvent()
{
  if(close(m_event) != 0)
    throw Exception("Failed to close event: " + lastError());
}

int LinuxEvent::getHandle() const
{
  return m_event;
}

void LinuxEvent::set()
{
  uint64_t state = 1;

  if(write(m_event, &state, sizeof(state)) != sizeof(state) && errno != EAGAIN)
    throw Exception("Failed to set event: " + lastError());
}

void LinuxEvent::reset()
{
  uint64_t state;

  if(read(m_event, &state, sizeof(state)) != sizeof(state) && errno != EAGAIN)
    throw Exception("Failed to reset event: " + lastError());
}
