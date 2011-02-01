#include "LinuxSemaphore.h"
#include "Exception.h"
#include "Abstraction.h"
#include <unistd.h>
#include "eventfd.h"

LinuxSemaphore::LinuxSemaphore(uint32_t maxCount __attribute__ ((unused)),
                               uint32_t initialCount) :
  m_semaphore(eventfd(initialCount, (EFD_SEMAPHORE | EFD_WAITREAD)))
{
  if(m_semaphore == -1)
    throw Exception("Failed to create semaphore: " + lastError());
}

LinuxSemaphore::~LinuxSemaphore()
{
  if(close(m_semaphore) != 0)
    throw Exception("Failed to close semaphore: " + lastError());
}

void LinuxSemaphore::lock(uint32_t timeout __attribute__ ((unused)))
{
  if(WaitForObject(m_semaphore, timeout) != WaitSuccess)
    throw Exception("Failed to lock semaphore: " + lastError());

  // Code to use eventfd without WAITREAD
  // uint64_t buffer;
  // if(read(m_semaphore, &buffer, sizeof(buffer)) != sizeof(buffer))
  //   throw Exception("Failed to read semaphore: " + lastError());
}

void LinuxSemaphore::unlock(uint32_t count)
{
  uint64_t internalCount(count);
  
  if(write(m_semaphore, &internalCount, sizeof(internalCount)) != sizeof(internalCount))
    throw Exception("Failed to unlock semaphore: " + lastError());
}

int LinuxSemaphore::getHandle()
{
  return m_semaphore;
}
