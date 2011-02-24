#include "linux/LinuxWaitSet.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include "mct/hash-map.hpp"
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

LinuxWaitSet::LinuxWaitSet() :
  m_waitObjects(new mct::closed_hash_map<Handle,
                                         WaitObject*,
                                         std::tr1::hash<Handle>,
                                         std::equal_to<Handle>,
                                         std::allocator<std::pair<const Handle, WaitObject*> >,
                                         false>),
  m_waitArray(NULL),
  m_eventOffset(0)
{
  // Do nothing
}

LinuxWaitSet::~LinuxWaitSet()
{
  delete [] m_waitArray;
  delete m_waitObjects;
}

bool LinuxWaitSet::add(WaitObject& obj)
{
  // Make sure handle is valid
  if(fcntl(obj.getHandle(), F_GETFL) == -1 && errno == EBADF)
    throw std::bad_syscall("fcntl", lastError());

  if(!m_waitObjects->insert(std::make_pair<Handle, WaitObject*>(obj.getHandle(), &obj)).second)
    return false;

  pollfd* oldArray = m_waitArray;
  m_waitArray = new pollfd[m_waitObjects->size()];

  // Copy over the old array
  uint32_t i = 0;
  for(; i < m_waitObjects->size() - 1; ++i)
    m_waitArray[i] = oldArray[i];

  // Add the new handle at the end of the array
  m_waitArray[i].fd = obj.getHandle();
  m_waitArray[i].events = POLLIN;
  m_waitArray[i].revents = 0;

  delete [] oldArray;

  return true;
}

bool LinuxWaitSet::remove(WaitObject& obj)
{
  return remove(obj.getHandle());
}

bool LinuxWaitSet::remove(Handle handle)
{
  if(!m_waitObjects->erase(handle))
    return false;

  pollfd* oldArray = m_waitArray;
  m_waitArray = new pollfd[m_waitObjects->size()];

  // Copy over the old array, skipping over the removed handle
  uint32_t j = 0;
  for(uint32_t i = 0; i < m_waitObjects->size() + 1; ++i)
  {
    if(oldArray[i].fd != handle)
      m_waitArray[j++] = oldArray[i];
    else
      m_eventOffset = i;
  }

  delete [] oldArray;

  return true;
}

size_t LinuxWaitSet::getSize() const
{
  return m_waitObjects->size();
}

WaitResult LinuxWaitSet::waitAny(uint32_t timeout, Handle& handle)
{
  uint32_t endTime = getTime() + timeout;

  if(m_waitObjects->size() == 0)
  {
    Sleep(timeout);
    return WaitTimeout;
  }

  WaitResult result = getEvent(handle);

  if(result == WaitTimeout)
  {
    std::list<Handle> preWaitEvents;

    for(mct::closed_hash_map<Handle, WaitObject*>::iterator i = m_waitObjects->begin();
        i != m_waitObjects->end(); ++i)
      if(i->second->preWaitCallback())
        preWaitEvents.push_back(i->second->getHandle());

    if(preWaitEvents.size() != 0)
    {
      result = pollEvents(0, endTime);
      addEvents(preWaitEvents);
    }
    else
      result = pollEvents(timeout, endTime);

    // Call postWaitCallback on all waitobjects
    for(uint32_t i = 0; i < m_waitObjects->size(); ++i)
    {
      if(m_waitArray[i].revents & (POLLERR | POLLHUP | POLLNVAL))
        m_waitObjects->at(m_waitArray[i].fd)->postWaitCallback(WaitAbandoned);
      else if(m_waitArray[i].revents & POLLIN)
        m_waitObjects->at(m_waitArray[i].fd)->postWaitCallback(WaitSuccess);
      else
        m_waitObjects->at(m_waitArray[i].fd)->postWaitCallback(WaitTimeout);
    }

    if(result == WaitError)
      throw std::bad_syscall("poll", lastError());
    if(result == WaitSuccess)
      result = getEvent(handle);
  }

  return result;
}

WaitResult LinuxWaitSet::pollEvents(uint32_t timeout, uint32_t endTime)
{
  WaitResult result = WaitTimeout;

  m_eventOffset = 0;

  do
  {
    int32_t eventCount = poll(m_waitArray, m_waitObjects->size(), timeout);

    if(eventCount == 0)
    {
      if(errno == EINTR)
      {
        uint32_t currentTime = getTime();
        timeout = (endTime <= currentTime ? 0 : endTime - currentTime);
        continue;
      }
      result = WaitTimeout;
    }
    else if(eventCount < 0)
      result = WaitError;
    else
      result = WaitSuccess;
  } while(false);

  return result;
}

void LinuxWaitSet::addEvents(const std::list<Handle>& events)
{
  for(std::list<Handle>::const_iterator i = events.begin(); i != events.end(); ++i)
    for(uint32_t j = 0; j < m_waitObjects->size(); ++j)
      if(m_waitArray[j].fd == *i)
      {
        m_waitArray[j].revents |= POLLIN;
        break;
      }
}

WaitResult LinuxWaitSet::getEvent(Handle& handle)
{
  WaitResult result = WaitTimeout;

  // Scan for the next valid handle in the waitArray
  while(m_eventOffset < m_waitObjects->size())
  {
    if(m_waitArray[m_eventOffset].revents & (POLLERR | POLLHUP | POLLNVAL))
    {
      result = WaitAbandoned;
      m_waitArray[m_eventOffset].revents &= ~(POLLERR | POLLHUP | POLLNVAL); // Mask out the flags we used
      break;
    }
    else if(m_waitArray[m_eventOffset].revents & POLLIN)
    {
      result = WaitSuccess;
      m_waitArray[m_eventOffset].revents = 0;
      break;
    }

    ++m_eventOffset;
  }

  if(result == WaitTimeout)
    handle = INVALID_HANDLE_VALUE;
  else
    handle = m_waitArray[m_eventOffset].fd;

  return result;
}

