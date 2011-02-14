#include "windows/WindowsWaitSet.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include "mct/hash-map.hpp"
#include <Windows.h>

WindowsWaitSet::WindowsWaitSet() :
  m_waitObjects(new mct::closed_hash_map<Handle, WaitObject*>),
  m_handleArray(NULL),
  m_offset(0)
{
  // Do nothing
}

WindowsWaitSet::~WindowsWaitSet()
{
  delete [] m_handleArray;
}

bool WindowsWaitSet::add(WaitObject& obj)
{
  DWORD handleInfo;

  if(obj.getHandle() == INVALID_HANDLE_VALUE ||
     !GetHandleInformation(obj.getHandle(), &handleInfo))
    throw std::invalid_argument("invalid handle");

  if(!m_waitObjects->insert(std::make_pair<Handle, WaitObject*>(obj.getHandle(), &obj)).second)
    return false;

  resizeEvents();
  return true;
}

bool WindowsWaitSet::remove(WaitObject& obj)
{
  return remove(obj.getHandle());
}

bool WindowsWaitSet::remove(Handle handle)
{
  DWORD handleInfo;

  if(!m_waitObjects->erase(handle))
    return false;

  resizeEvents();
  return true;
}

size_t WindowsWaitSet::getSize() const
{
  return m_waitObjects->size();
}

WaitResult WindowsWaitSet::waitAll(uint32_t timeout, Handle& handle)
{
  DWORD result(WaitForMultipleObjects(m_waitObjects->size(), m_handleArray, true, timeout));

  if(result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + m_waitObjects->size())
  {
    handle = INVALID_HANDLE_VALUE;
    return WaitSuccess;
  }
  else if(result >= WAIT_ABANDONED_0 && result < WAIT_ABANDONED_0 + m_waitObjects->size())
  {
    handle = m_handleArray[result - WAIT_ABANDONED_0];
    return WaitAbandoned;
  }
  else if(result == WAIT_TIMEOUT)
  {
    handle = INVALID_HANDLE_VALUE;
    return WaitTimeout;
  }

  throw std::bad_syscall("WaitForMultipleObjects", lastError());
}

WaitResult WindowsWaitSet::waitAny(uint32_t timeout, Handle& handle)
{
  WaitResult result;
  DWORD retval(WaitForMultipleObjects(m_waitObjects->size(), m_handleArray + m_offset, false, timeout));

  if(retval >= WAIT_OBJECT_0 && retval < WAIT_OBJECT_0 + m_waitObjects->size())
  {
    handle = m_handleArray[retval - WAIT_OBJECT_0 + m_offset];
    m_offset = (m_offset + retval - WAIT_OBJECT_0 + 1) % m_waitObjects->size();
    result = WaitSuccess;
  }
  else if(retval >= WAIT_ABANDONED_0 && retval < WAIT_ABANDONED_0 + m_waitObjects->size())
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
    throw std::bad_syscall("WaitForMultipleObjects", lastError());

  (*m_waitObjects)[handle]->postWaitCallback(result);
  return result;
}

void WindowsWaitSet::resizeEvents()
{
  Handle favoredHandle = INVALID_HANDLE_VALUE;

  if(m_handleArray != NULL)
  {
    favoredHandle = m_handleArray[m_offset];
    delete [] m_handleArray;
  }

  // The array is doubled to allow for a sliding window of Handles in order to
  //  avoid unfairness in the wait
  m_handleArray = new Handle[m_waitObjects->size() * 2];

  uint32_t j(0);
  for(mct::closed_hash_map<Handle, WaitObject*>::iterator i(m_waitObjects->begin());
      i != m_waitObjects->end(); ++i)
    m_handleArray[j++] = i->first;

  for(mct::closed_hash_map<Handle, WaitObject*>::iterator i(m_waitObjects->begin());
      i != m_waitObjects->end(); ++i)
    m_handleArray[j++] = i->first;

  // Avoid resetting the unfairness protection, try to find where the new offset should be
  for(uint32_t i(0); i < m_waitObjects->size(); ++i)
  {
    if(m_handleArray[i] == favoredHandle)
      m_offset = i;
  }

  // The favored handle must have been removed, just make sure the offset isn't out of bounds
  if(m_waitObjects->size() != 0)
    m_offset = m_offset % m_waitObjects->size();
}
