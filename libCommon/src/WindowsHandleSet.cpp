#include "WindowsHandleSet.h"
#include "stdint.h"

WindowsHandleSet::WindowsHandleSet() :
  m_handleArray(NULL),
  m_offset(0)
{
  // Do nothing
}

WindowsHandleSet::~WindowsHandleSet()
{
  delete [] m_handleArray;
}

bool WindowsHandleSet::add(HANDLE handle)
{
  if(m_handleSet.find(handle) != m_handleSet.end())
    return false;

  m_handleSet.insert(handle);
  resizeEvents();
  return true;
}

bool WindowsHandleSet::remove(HANDLE handle)
{
  if(m_handleSet.find(handle) == m_handleSet.end())
    return false;

  m_handleSet.erase(handle);
  resizeEvents();
  return true;
}

uint32_t WindowsHandleSet::getSize() const
{
  return m_handleSet.size();
}

const std::set<HANDLE>& WindowsHandleSet::getSet() const
{
  return m_handleSet;
}

int WindowsHandleSet::waitAll(uint32_t timeout, HANDLE& handle)
{
  DWORD result = WaitForMultipleObjects(m_handleSet.size(), m_handleArray, true, timeout);

  if(result >= WAIT_ABANDONED_0 && result < WAIT_ABANDONED_0 + m_handleSet.size())
  {
    handle = m_handleArray[result - WAIT_ABANDONED_0];
    return WaitAbandoned;
  }
  else if(result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + m_handleSet.size())
  {
    handle = INVALID_HANDLE_VALUE;
    return WaitSuccess;
  }
  else if(result == WAIT_TIMEOUT)
  {
    handle = INVALID_HANDLE_VALUE;
    return WaitTimeout;
  }

  handle = INVALID_HANDLE_VALUE;
  return WaitError;
}

int WindowsHandleSet::waitAny(uint32_t timeout, HANDLE& handle)
{
  DWORD result = WaitForMultipleObjects(m_handleSet.size(), m_handleArray + m_offset, false, timeout);

  if(result >= WAIT_ABANDONED_0 && result < WAIT_ABANDONED_0 + m_handleSet.size())
  {
    handle = m_handleArray[result - WAIT_ABANDONED_0 + m_offset];
    return WaitAbandoned;
  }
  else if(result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + m_handleSet.size())
  {
    handle = m_handleArray[result - WAIT_OBJECT_0 + m_offset];
    m_offset = (m_offset + result - WAIT_OBJECT_0 + 1) % m_handleSet.size();
    return WaitSuccess;
  }
  else if(result == WAIT_TIMEOUT)
  {
    handle = INVALID_HANDLE_VALUE;
    return WaitTimeout;
  }

  handle = INVALID_HANDLE_VALUE;
  return WaitError;
}

void WindowsHandleSet::resizeEvents()
{
  delete [] m_handleArray;

  // The array is doubled to allow for a sliding window of HANDLEs, to avoid unfairness in the wait
  m_handleArray = new HANDLE[m_handleSet.size() * 2];

  uint32_t j = 0;
  for(std::set<HANDLE>::iterator i = m_handleSet.begin(); i != m_handleSet.end(); ++i)
    m_handleArray[j++] = *i;

  for(std::set<HANDLE>::iterator i = m_handleSet.begin(); i != m_handleSet.end(); ++i)
    m_handleArray[j++] = *i;

  // TODO: probably better not to reset, can this be avoided?
  m_offset = 0;
}
