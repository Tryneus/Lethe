#include "windows/WindowsMutex.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include <Windows.h>

WindowsMutex::WindowsMutex(bool locked) :
  WaitObject(CreateMutex(NULL, locked, NULL))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("CreateMutex", lastError());
}

WindowsMutex::~WindowsMutex()
{
  CloseHandle(getHandle());
}

void WindowsMutex::lock(uint32_t timeout)
{
  if(WaitForObject(*this, timeout) != WaitSuccess)
    throw std::runtime_error("failed to wait for mutex");
}

void WindowsMutex::unlock()
{
  if(!ReleaseMutex(getHandle()))
    throw std::bad_syscall("ReleaseMutex", lastError());
}
