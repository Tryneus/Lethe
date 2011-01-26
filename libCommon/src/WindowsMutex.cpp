#include "WindowsMutex.h"
#include "Exception.h"
#include "Abstraction.h"
#include "Windows.h"
#include <sstream>

WindowsMutex::WindowsMutex(bool locked) :
  m_handle(CreateMutex(NULL, locked, NULL))
{
  if(m_handle == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create mutex: " + lastError());
}

WindowsMutex::~WindowsMutex()
{
  if(!CloseHandle(m_handle))
    throw Exception("Failed to close mutex: " + lastError());
}

void WindowsMutex::lock(uint32_t timeout)
{
  if(WaitForObject(m_handle, timeout) != WaitSuccess)
    throw Exception("Failed to lock mutex: " + lastError());
}

void WindowsMutex::unlock()
{
  if(!ReleaseMutex(m_handle))
    throw Exception("Failed to unlock mutex: " + lastError());
}


