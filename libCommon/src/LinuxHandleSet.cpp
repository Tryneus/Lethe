#include "linux/LinuxHandleSet.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <sys/epoll.h>
#include <string.h>

LinuxHandleSet::LinuxHandleSet() :
  m_epollSet(epoll_create(10)),
  m_events(NULL),
  m_eventCount(0)
{
  if(m_epollSet == -1)
    throw Exception("Failed to create epoll set: " + lastError());
}

LinuxHandleSet::~LinuxHandleSet()
{
  delete [] m_events;

  if(close(m_epollSet) != 0)
    throw Exception("Failed to close epoll set: " + lastError());
}

void LinuxHandleSet::add(int fd)
{
  if(m_fdSet.find(fd) != m_fdSet.end())
    throw Exception("Failed to find handle in set");

  epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = EPOLLIN;
  event.data.fd = fd;

  if(epoll_ctl(m_epollSet, EPOLL_CTL_ADD, fd, &event) != 0)
    throw Exception("Failed to add handle to set: " + lastError());

  m_fdSet.insert(fd);
  resizeEvents();
}

void LinuxHandleSet::remove(int fd)
{
  if(m_fdSet.find(fd) == m_fdSet.end())
    throw Exception("Failed to find handle in set");

  epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = fd;

  if(epoll_ctl(m_epollSet, EPOLL_CTL_DEL, fd, &event) != 0)
    throw Exception("Failed to remove handle from set: " + lastError());

  m_fdSet.erase(fd);
  resizeEvents();
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

int LinuxHandleSet::waitAll(uint32_t timeout __attribute__ ((unused)), 
                            int& fd __attribute__ ((unused)))
{
  throw Exception("LinuxHandleSet::waitAll not yet implemented");
}

int LinuxHandleSet::waitAny(uint32_t timeout, int& fd)
{
  // TODO: save end time, and rewait if EINTR

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
      throw Exception("Failed to wait: " + lastError());
    }
  }

  fd = m_events[--m_eventCount].data.fd;
  
  if(m_events[m_eventCount].events & (EPOLLERR | EPOLLHUP))
    return WaitAbandoned;

  return WaitSuccess;
}

