#include "windows/WindowsSemaphore.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <Windows.h>

WindowsSemaphore::WindowsSemaphore(uint32_t maxCount, uint32_t initialCount) :
  m_semaphore(CreateSemaphore(NULL, initialCount, maxCount, NULL))
{
  if(m_semaphore == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create semaphore: " + lastError());
}

WindowsSemaphore::~WindowsSemaphore()
{
  if(!CloseHandle(m_semaphore))
    throw Exception("Failed to close semaphore: " + lastError());
}

void WindowsSemaphore::lock(uint32_t timeout)
{
  if(WaitForObject(m_semaphore, timeout) != WaitSuccess)
    throw Exception("Failed to lock semaphore: " + lastError());
}

void WindowsSemaphore::unlock(uint32_t count)
{
  if(!ReleaseSemaphore(m_semaphore, count, NULL))
    throw Exception("Failed to release semaphore: " + lastError());
}
