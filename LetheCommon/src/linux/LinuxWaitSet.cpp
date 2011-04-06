#include "linux/LinuxWaitSet.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include "LetheInternal.h"
#include "mct/hash-map.hpp"
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

using namespace lethe;

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

  if(!m_waitObjects->insert(std::make_pair(obj.getHandle(), &obj)).second)
    return false;

  pollfd* oldArray = m_waitArray;
  m_waitArray = new pollfd[m_waitObjects->size()];
  m_preWaitEvents.reserve(m_waitObjects->size());

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
  m_preWaitEvents.reserve(m_waitObjects->size());

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
  uint32_t endTime = getEndTime(timeout);

  if(m_waitObjects->size() == 0)
  {
    Sleep(timeout);
    return WaitTimeout;
  }

  WaitResult result = getEvent(handle);

  if(result == WaitTimeout)
  {
    m_preWaitEvents.clear();

    for(auto i = m_waitObjects->begin(); i != m_waitObjects->cend(); ++i)
      if(i->second->preWaitCallback())
        m_preWaitEvents.push_back(i->second->getHandle());

    if(!m_preWaitEvents.empty())
    {
      result = pollEvents(0, endTime);
      addEvents(m_preWaitEvents);
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

  while(true)
  {
    int32_t eventCount = poll(m_waitArray, m_waitObjects->size(), timeout);

    if(eventCount < 0)
    {
      if(errno == EINTR)
      {
        timeout = getTimeout(endTime);
        continue;
      }
      else if(errno == EBADF)
      {
        findBadHandles();
        result = WaitSuccess;
      }
      else
        result = WaitError;
    }
    else if(eventCount == 0)
      result = WaitTimeout;
    else
      result = WaitSuccess;

    break;
  }

  return result;
}

// TODO: get this working, it isn't
void LinuxWaitSet::findBadHandles()
{
  for(auto i = m_waitObjects->cbegin(); i != m_waitObjects->cend(); ++i)
  {
    if(fcntl(i->first, F_GETFL) == -1 && errno == EBADF)
    {
      for(uint32_t j = 0; j < m_waitObjects->size(); ++j)
      {
        if(m_waitArray[j].fd == i->first)
        {
          m_waitArray[j].revents |= POLLERR;
          if(j < m_eventOffset)
            m_eventOffset = j;
          break;
        }
      }
    }
  }
}

void LinuxWaitSet::addEvents(const std::vector<Handle>& events)
{
  for(auto i = events.cbegin(); i != events.cend(); ++i)
    for(uint32_t j = 0; j < m_waitObjects->size(); ++j)
      if(m_waitArray[j].fd == *i)
      {
        m_waitArray[j].revents |= POLLIN;
        if(j < m_eventOffset)
          m_eventOffset = j;
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
      // m_waitArray[m_eventOffset].revents &= ~(POLLERR | POLLHUP | POLLNVAL); // Mask out the flags we used
      m_waitArray[m_eventOffset].revents = 0; // Mask out all events, error conditions prevent success
      result = WaitAbandoned;
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

