#include "windows/WindowsWaitSet.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <Windows.h>

WindowsWaitSet::WindowsWaitSet() :
  m_handleArray(NULL),
  m_offset(0)
{
  // Do nothing
}

WindowsWaitSet::~WindowsWaitSet()
{
  delete [] m_handleArray;
}

void WindowsWaitSet::add(Handle handle)
{
  if(m_waitObjects.find(handle) != m_waitObjects.end())
    throw Exception("Failed to find handle in set");

  if(!m_waitObjects.insert(handle).second)
    throw Exception("Failed to add handle to set");

  resizeEvents();
}

void WindowsWaitSet::remove(Handle handle)
{
  if(m_waitObjects.find(handle) == m_waitObjects.end())
    throw Exception("Failed to find handle in set");

  if(!m_waitObjects.erase(handle))
    throw Exception("Failed to remove handle from set");

  resizeEvents();
}

size_t WindowsWaitSet::getSize() const
{
  return m_waitObjects.size();
}

WaitResult WindowsWaitSet::waitAll(uint32_t timeout, Handle& handle)
{
  DWORD result(WaitForMultipleObjects(m_waitObjects.size(), m_handleArray, true, timeout));

  if(result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + m_waitObjects.size())
  {
    handle = INVALID_HANDLE_VALUE;
    return WaitSuccess;
  }
  else if(result >= WAIT_ABANDONED_0 && result < WAIT_ABANDONED_0 + m_waitObjects.size())
  {
    handle = m_handleArray[result - WAIT_ABANDONED_0];
    return WaitAbandoned;
  }
  else if(result == WAIT_TIMEOUT)
  {
    handle = INVALID_HANDLE_VALUE;
    return WaitTimeout;
  }

  throw Exception("Failed to wait: " + lastError());
}

WaitResult WindowsWaitSet::waitAny(uint32_t timeout, Handle& handle)
{
  WaitResult result;
  DWORD retval(WaitForMultipleObjects(m_waitObjects.size(), m_handleArray + m_offset, false, timeout));

  if(retval >= WAIT_OBJECT_0 && retval < WAIT_OBJECT_0 + m_waitObjects.size())
  {
    handle = m_handleArray[retval - WAIT_OBJECT_0 + m_offset];
    m_offset = (m_offset + retval - WAIT_OBJECT_0 + 1) % m_waitObjects.size();
    result = WaitSuccess;
  }
  else if(retval >= WAIT_ABANDONED_0 && retval < WAIT_ABANDONED_0 + m_waitObjects.size())
  {
    handle = m_handleArray[retval - WAIT_ABANDONED_0 + m_offset];
    result = WaitAbandoned;
  }
  else if(retval == WAIT_TIMEOUT)
  {
    handle = INVALID_HANDLE_VALUE;
    return WaitTimeout;
  }
  else
    throw Exception("Failed to wait: " + lastError());

  m_waitObjects[handle]->postWaitCallback(result);
  return result;
}

void WindowsWaitSet::resizeEvents()
{
  Handle favoredHandle = m_handleArray[m_offset];

  delete [] m_handleArray;

  // The array is doubled to allow for a sliding window of Handles in order to
  //  avoid unfairness in the wait
  m_handleArray = new Handle[m_waitObjects.size() * 2];

  uint32_t j(0);
  for(std::set<Handle>::iterator i(m_waitObjects.begin()); i != m_waitObjects.end(); ++i)
    m_handleArray[j++] = *i;

  for(std::set<Handle>::iterator i(m_waitObjects.begin()); i != m_waitObjects.end(); ++i)
    m_handleArray[j++] = *i;

  // Avoid resetting the unfairness protection, try to find where the new offset should be
  for(uint32_t i(0); i < m_waitObjects.size(); ++i)
  {
    if(m_handleArray[i] == favoredHandle)
      m_offset = i;
  }

  // The favored handle must have been removed, just make sure the offset isn't out of bounds
  m_offset = m_offset % m_waitObjects.size();
}
