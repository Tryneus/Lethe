#include "linux/LinuxSemaphore.h"
#include "AbstractionTypes.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include "eventfd.h"

LinuxSemaphore::LinuxSemaphore(uint32_t maxCount __attribute__ ((unused)),
                               uint32_t initialCount) :
  WaitObject(eventfd(initialCount, (EFD_NONBLOCK | EFD_SEMAPHORE | EFD_WAITREAD)))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create semaphore: " + lastError());
}

LinuxSemaphore::~LinuxSemaphore()
{
  close(getHandle());
}

void LinuxSemaphore::lock(uint32_t timeout)
{
  if(WaitForObject(*this, timeout) != WaitSuccess)
    throw Exception("Failed to lock semaphore: " + lastError());
}

void LinuxSemaphore::unlock(uint32_t count)
{
  uint64_t internalCount(count);

  if(write(getHandle(), &internalCount, sizeof(internalCount)) != sizeof(internalCount))
    throw Exception("Failed to unlock semaphore: " + lastError());
}
