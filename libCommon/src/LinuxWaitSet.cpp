#include "linux/LinuxWaitSet.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include "mct/hash-map.hpp"
#include <string.h>
#include <errno.h>

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
    throw std::bad_syscall("epoll_create", lastError());
}

LinuxWaitSet::~LinuxWaitSet()
{
  delete [] m_events;
  delete m_waitObjects;

  close(m_epollSet);
}

bool LinuxWaitSet::add(WaitObject& obj)
{
  if(!m_waitObjects->insert(std::make_pair<Handle, WaitObject*>(obj.getHandle(), &obj)).second)
    return false;

  epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
  event.data.fd = obj.getHandle();

  if(epoll_ctl(m_epollSet, EPOLL_CTL_ADD, event.data.fd, &event) != 0)
  {
    m_waitObjects->erase(obj.getHandle());
    throw std::bad_syscall("epoll_ctl", lastError());
  }

  resizeEvents();
  return true;
}

bool LinuxWaitSet::remove(WaitObject& obj)
{
  return remove(obj.getHandle());
}

bool LinuxWaitSet::remove(Handle handle)
{
  bool retval = m_waitObjects->erase(handle);

  epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
  event.data.fd = handle;

  try
  {
    if(epoll_ctl(m_epollSet, EPOLL_CTL_DEL, event.data.fd, &event) != 0 &&
       errno != ENOENT &&
       errno != EBADF)
      throw std::bad_syscall("epoll_ctl", lastError());
  }
  catch(...)
  {
    resizeEvents();
    throw;
  }

  resizeEvents();

  return retval;
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

WaitResult LinuxWaitSet::waitAll(uint32_t timeout GCC_UNUSED,
                                 Handle& handle GCC_UNUSED)
{
  // TODO: implement waitAll, may be too prone to deadlock, though
  throw std::logic_error("waitAll is not implemented on this platform");
}

WaitResult LinuxWaitSet::waitAny(uint32_t timeout, Handle& handle)
{
  // TODO: save end time and rewait if EINTR

  if(m_waitObjects->size() == 0)
  {
    Sleep(timeout);
    return WaitTimeout;
  }

  if(m_eventCount == 0)
  {
    std::list<Handle> preWaitEvents;

    for(mct::closed_hash_map<Handle, WaitObject*>::iterator i = m_waitObjects->begin();
        i != m_waitObjects->end(); ++i)
      if(i->second->preWaitCallback())
        preWaitEvents.push_back(i->second->getHandle());

    if(preWaitEvents.size() != 0)
      timeout = 0;

    if(m_eventCount == 0)
    {
      m_eventCount = epoll_wait(m_epollSet, m_events, m_waitObjects->size(), timeout);

      if(m_eventCount == 0)
      {
        handle = INVALID_HANDLE_VALUE;
        return WaitTimeout;
      }
      else if(m_eventCount < 0)
        throw std::bad_syscall("epoll_wait", lastError());
    }

    if(preWaitEvents.size() != 0)
      appendEvents(preWaitEvents);
  }

  return getEvent(handle);
}

void LinuxWaitSet::appendEvents(const std::list<Handle>& events)
{
  for(std::list<Handle>::const_iterator i = events.begin(); i != events.end(); ++i)
  {
    m_events[m_eventCount].data.fd = *i;
    m_events[m_eventCount++].events = EPOLLIN;
  }
}

WaitResult LinuxWaitSet::getEvent(Handle& handle)
{
  WaitResult result = WaitSuccess;
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

