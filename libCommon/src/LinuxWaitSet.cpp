#include "linux/LinuxWaitSet.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include "mct/hash-map.hpp"
#include <sys/epoll.h>
#include <string.h>

LinuxWaitSet::LinuxWaitSet() :
  m_epollSet(epoll_create(10)),
  m_waitObjects(new mct::closed_hash_map<Handle,
                                         WaitObject*,
                                         std::tr1::hash<Handle>,
                                         std::equal_to<Handle>,
                                         std::allocator<std::pair<const Handle, WaitObject*> >,
                                         false>),
  m_events(NULL),
  m_eventCount(0)
{
  if(m_epollSet == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create epoll set: " + lastError());
}

LinuxWaitSet::~LinuxWaitSet()
{
  delete [] m_events;
  delete m_waitObjects;

  if(close(m_epollSet) != 0)
    throw Exception("Failed to close epoll set: " + lastError());
}

void LinuxWaitSet::add(WaitObject& obj)
{
  if(!m_waitObjects->insert(std::make_pair<Handle, WaitObject*>(obj.getHandle(), &obj)).second)
    throw Exception("Failed to insert wait object into hash map");

  epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
  event.data.fd = obj.getHandle();

  if(epoll_ctl(m_epollSet, EPOLL_CTL_ADD, event.data.fd, &event) != 0)
  {
    m_waitObjects->erase(obj.getHandle());
    throw Exception("Failed to add handle to set: " + lastError());
  }

  resizeEvents();
}

void LinuxWaitSet::remove(WaitObject& obj)
{
  remove(obj.getHandle());
}

void LinuxWaitSet::remove(Handle handle)
{
  std::string error;

  if(!m_waitObjects->erase(handle))
    error += "Failed to remove handle from hash map";

  epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
  event.data.fd = handle;

  if(epoll_ctl(m_epollSet, EPOLL_CTL_DEL, event.data.fd, &event) != 0)
  {
    if(!error.empty())
      error.append(", and");
    error += "Failed to remove handle from epoll set: " + lastError();
  }

  resizeEvents();

  if(error.length() != 0)
    throw Exception(error);
}

void LinuxWaitSet::resizeEvents()
{
  epoll_event* oldEvents(m_events);
  m_events = new epoll_event[m_waitObjects->size()];

  // Copy any unprocessed events over and verify each handle
  uint32_t j(0);
  for(int i(0); i < m_eventCount; ++i)
    if(m_waitObjects->find(oldEvents[i].data.fd) != m_waitObjects->end())
      m_events[j++] = oldEvents[i];

  m_eventCount = j;
  
  delete [] oldEvents;
}

size_t LinuxWaitSet::getSize() const
{
  return m_waitObjects->size();
}

WaitResult LinuxWaitSet::waitAll(uint32_t timeout __attribute__ ((unused)), 
                                   Handle& handle __attribute__ ((unused)))
{
  // TODO: implement waitAll, may be too prone to deadlock, though
  throw Exception("LinuxWaitSet::waitAll not yet implemented");
}

WaitResult LinuxWaitSet::waitAny(uint32_t timeout, Handle& handle)
{
  WaitResult result(WaitSuccess);
  // TODO: save end time, and rewait if EINTR

  if(m_eventCount == 0)
  {
    m_eventCount = epoll_wait(m_epollSet, m_events, m_waitObjects->size(), timeout);

    if(m_eventCount == 0)
    {
      handle = INVALID_HANDLE_VALUE;
      return WaitTimeout;
    }
    else if(m_eventCount < 0)
      throw Exception("Failed to wait: " + lastError());
  }

  handle = m_events[--m_eventCount].data.fd;

  // In the case of an error, return WaitAbandoned
  if(m_events[m_eventCount].events & (EPOLLERR | EPOLLHUP))
  {
    // If the wait was also successful, hold onto that part of the event to be
    //  handled later (if the wait object isn't removed)
    if(m_events[m_eventCount].events & EPOLLIN)
      m_events[m_eventCount++].events = EPOLLIN;

    result = WaitAbandoned;
  }

  (*m_waitObjects)[handle]->postWaitCallback(result);
  return result;
}

