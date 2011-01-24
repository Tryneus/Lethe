#include "LinuxSemaphore.h"
#include "Abstraction.h"

LinuxSemaphore(uint32_t maxCount, uint32_t initialCount) :
  m_semaphore(eventfd(initialCount, EFD_SEMAPHORE | EFD_NONBLOCK))
{
  if(m_semaphore == -1)
    throw Exception("Failed to create Semaphore: " + lastError());
}

~LinuxSemaphore()
{
  close(m_semaphore);
}

void lock()
{
  uint64_t buffer;
  
  if(read(m_semaphore, &buffer, sizeof(buffer)) != sizeof(buffer))
    throw Exception("Failed to lock semaphore: " + lastError());
}

void unlock(uint32_t count)
{
  uint64_t internalCount(count);
  
  if(write(m_semaphore, &internalCount, sizeof(internalCount)) != sizeof(internalCount))
    throw Exception("Failed to unlock semaphore: " + lastError());
}

int getHandle()
{
  return m_semaphore;
}
