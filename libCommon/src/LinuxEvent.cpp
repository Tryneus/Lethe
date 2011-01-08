#include "LinuxEvent.h"
#include "Exception.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

LinuxEvent::LinuxEvent(bool initialState)
{
  int fds[2];

  if(pipe(fds) == -1)
    throw Exception("Failed to create Event pipe");

  m_readFd = fds[0];
  m_writeFd = fds[1];

  fcntl(m_readFd, F_SETFL, O_NONBLOCK);
  fcntl(m_writeFd, F_SETFL, O_NONBLOCK);

  if(initialState)
  {
    set();
  }
}

LinuxEvent::~LinuxEvent()
{
  close(m_writeFd);
  close(m_readFd);
}

int LinuxEvent::getHandle() const
{
  return m_readFd;
}

void LinuxEvent::set()
{
  if(write(m_writeFd, "S", 1) != 1)
    throw Exception("Failed to write to Event pipe");
}

void LinuxEvent::reset()
{
  // TODO: make sure this works
  char buffer[100];
  while(read(m_readFd, buffer, 100) == 100) { };
}
