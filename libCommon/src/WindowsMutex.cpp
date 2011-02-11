#include "windows/WindowsMutex.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <Windows.h>

WindowsMutex::WindowsMutex(bool locked) :
  WaitObject(CreateMutex(NULL, locked, NULL))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create mutex: " + lastError());
}

WindowsMutex::~WindowsMutex()
{
  CloseHandle(getHandle());
}

void WindowsMutex::lock(uint32_t timeout)
{
  if(WaitForObject(*this, timeout) != WaitSuccess)
    throw Exception("Failed to lock mutex: " + lastError());
}

void WindowsMutex::unlock()
{
  if(!ReleaseMutex(getHandle()))
    throw Exception("Failed to unlock mutex: " + lastError());
}
