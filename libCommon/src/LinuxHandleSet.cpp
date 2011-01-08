#include "LinuxHandleSet.h"
#include "Exception.h"
#include <errno.h>

LinuxHandleSet::LinuxHandleSet() :
  m_epollSet(epoll_create(10)),
  m_events(NULL),
  m_eventCount(0)
{
  if(m_epollSet == -1)
    throw Exception("Failed to create epoll set");
}

LinuxHandleSet::~LinuxHandleSet()
{
  delete [] m_events;
  close(m_epollSet);
}

bool LinuxHandleSet::add(int fd)
{
  if(m_fdSet.find(fd) != m_fdSet.end())
    return false;

  epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = fd;
  epoll_ctl(m_epollSet, EPOLL_CTL_ADD, fd, &event);

  m_fdSet.insert(fd);
  resizeEvents();

  return true;
}

bool LinuxHandleSet::remove(int fd)
{
  if(m_fdSet.find(fd) == m_fdSet.end())
    return false;

  epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = fd;
  epoll_ctl(m_epollSet, EPOLL_CTL_DEL, fd, &event);

  m_fdSet.erase(fd);
  resizeEvents();

  return true;
}

void LinuxHandleSet::resizeEvents()
{
  epoll_event* oldEvents = m_events;
  m_events = new epoll_event[m_fdSet.size()];

  // Copy any unprocessed events over and verify each fd
  uint32_t j = 0;
  for(int i = 0; i < m_eventCount; ++i)
  {
    if(m_fdSet.find(oldEvents[i].data.fd) != m_fdSet.end())
    {
      m_events[j] = oldEvents[i];
      ++j;
    }
  
    ++i;
  }
  
  delete [] oldEvents;
}

uint32_t LinuxHandleSet::getSize() const
{
  return m_fdSet.size();
}

int LinuxHandleSet::waitAll(uint32_t timeout, int& fd)
{
  throw Exception("LinuxHandleSet::waitAll not yet implemented");
}

int LinuxHandleSet::waitAny(uint32_t timeout, int& fd)
{
  if(m_eventCount == 0)
  {
    m_eventCount = epoll_wait(m_epollSet, m_events, m_fdSet.size(), timeout);

    if(m_eventCount == 0)
    {
      fd = INVALID_HANDLE_VALUE;
      return WaitTimeout;
    }
    else if(m_eventCount < 0)
    {
      fd = INVALID_HANDLE_VALUE;
      return WaitError;
    }
  }

  fd = m_events[--m_eventCount].data.fd;
  
  if(m_events[m_eventCount].events & (EPOLLERR | EPOLLHUP))
    return WaitAbandoned;

  return WaitSuccess;
}

