#include "WindowsSemaphore.h"
#include "Abstraction.h"
#include "Exception.h"

WindowsSemaphore::WindowsSemaphore(uint32_t maxCount, uint32_t initialCount) :
  m_semaphore(CreateSemaphore(NULL, initialCount, maxCount, NULL))
{
  if(m_semaphore == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create Semaphore: " + lastError());
}

WindowsSemaphore::~WindowsSemaphore()
{
  CloseHandle(m_semaphore);
}

void WindowsSemaphore::lock()
{
  // TODO: Uneven behavior across platforms, this is done at the wait on the handle
}

void WindowsSemaphore::unlock(uint32_t count)
{
  if(!ReleaseSemaphore(m_semaphore, count, NULL))
    throw Exception("Failed to release semaphore: " + lastError());
}

HANDLE WindowsSemaphore::getHandle()
{
  return m_semaphore;
}
