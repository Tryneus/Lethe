#include "windows/WindowsSemaphore.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include <Windows.h>

WindowsSemaphore::WindowsSemaphore(uint32_t maxCount, uint32_t initialCount) :
  WaitObject(CreateSemaphore(NULL, initialCount, maxCount, NULL))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("CreateSemaphore", lastError());
}

WindowsSemaphore::~WindowsSemaphore()
{
  CloseHandle(getHandle());
}

void WindowsSemaphore::lock(uint32_t timeout)
{
  if(WaitForObject(*this, timeout) != WaitSuccess)
    throw std::runtime_error("failed to wait for semaphore");
}

void WindowsSemaphore::unlock(uint32_t count)
{
  if(!ReleaseSemaphore(getHandle(), count, NULL))
    throw std::bad_syscall("ReleaseSemaphore", lastError());
}
