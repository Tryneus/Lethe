#include "windows/WindowsMutex.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <Windows.h>

WindowsMutex::WindowsMutex(bool locked) :
  WaitObject(CreateMutex(NULL, locked, NULL)),
  m_ownerThread(locked ? GetCurrentThreadId() : -1)
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create mutex: " + lastError());
}

WindowsMutex::~WindowsMutex()
{
  if(!CloseHandle(getHandle()))
    throw Exception("Failed to close mutex: " + lastError());
}

void WindowsMutex::lock(uint32_t timeout)
{
  if(m_ownerThread != GetCurrentThreadId())
  {
    if(WaitForObject(getHandle(), timeout) != WaitSuccess)
      throw Exception("Failed to lock mutex: " + lastError());
    m_ownerThread = GetCurrentThreadId();
  }
}

void WindowsMutex::unlock()
{
  if(m_ownerThread == GetCurrentThreadId())
  {
    m_ownerThread = -1;

    if(!ReleaseMutex(getHandle()))
      throw Exception("Failed to unlock mutex: " + lastError());
  }
  else
    throw Exception("Failed to unlock mutex: locked by a different thread");
}

void WindowsMutex::postWaitCallback(WaitResult result)
{
  if(result == WaitSuccess)
    m_ownerThread = GetCurrentThreadId();
}
