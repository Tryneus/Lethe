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

const uint32_t LinuxWaitSet::s_maxWaitObjects(64);

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
  // Make sure there is size for the new wait object
  if(m_waitObjects->size() >= s_maxWaitObjects)
    return false;

  // Make sure handle is valid
  if(fcntl(obj.getHandle(), F_GETFL) == -1 && errno == EBADF)
    throw std::invalid_argument("invalid handle");

  if(!m_waitObjects->insert(std::make_pair(obj.getHandle(), &obj)).second)
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
  uint32_t endTime = getEndTime(timeout);

  if(m_waitObjects->size() == 0)
  {
    sleep_ms(timeout);
    return WaitTimeout;
  }

  WaitResult result = getEvent(handle);

  if(result == WaitTimeout)
  {
    if(pollEvents(timeout, endTime) == WaitSuccess)
      result = getEvent(handle);
    else
      handle = INVALID_HANDLE_VALUE;
  }

  return result;
}

WaitResult LinuxWaitSet::pollEvents(uint32_t timeout, uint32_t endTime)
{
  m_eventOffset = 0;

  do
  {
    int eventCount = poll(m_waitArray, m_waitObjects->size(), timeout);

    if(eventCount < 0)
    {
      if(errno == EINTR)
      {
        timeout = getTimeout(endTime);
        continue;
      }
      else
        throw std::bad_syscall("poll", lastError());
    }
    else if(eventCount == 0)
      return WaitTimeout;
    else
      return WaitSuccess;
  } while(true);
}

WaitResult LinuxWaitSet::getEvent(Handle& handle)
{
  WaitResult result = WaitTimeout;
  handle = INVALID_HANDLE_VALUE;

  // Scan for the next valid handle in the waitArray
  for(; m_eventOffset < m_waitObjects->size(); ++m_eventOffset)
  {
    // TODO: handled abandoned mutex event
    if(m_waitArray[m_eventOffset].revents & (POLLERR | POLLHUP | POLLNVAL))
    {
      m_waitArray[m_eventOffset].revents = 0; // Mask out all events, error conditions prevent success
      handle = m_waitArray[m_eventOffset].fd;
      result = WaitError;
      break;
    }
    else if(m_waitArray[m_eventOffset].revents & POLLIN)
    {
      m_waitArray[m_eventOffset].revents = 0;
      handle = m_waitArray[m_eventOffset].fd;
      result = WaitSuccess;
      break;
    }
  }

  return result;
}

