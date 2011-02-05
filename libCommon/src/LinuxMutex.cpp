#include "linux/LinuxMutex.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <pthread.h>
#include "eventfd.h"

LinuxMutex::LinuxMutex(bool locked) :
  WaitObject(eventfd((locked ? 0 : 1), (EFD_NONBLOCK | EFD_SEMAPHORE | EFD_WAITREAD))),
  m_ownerThread(locked ? pthread_self() : -1)
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create mutex: " + lastError());
}

LinuxMutex::~LinuxMutex()
{
  close(getHandle());
}

void LinuxMutex::lock(uint32_t timeout)
{
  if(m_ownerThread != pthread_self())
  {
    if(WaitForObject(*this, timeout) != WaitSuccess)
      throw Exception("Failed to lock mutex: " + lastError());

    m_ownerThread = pthread_self();
  }
}

void LinuxMutex::unlock()
{
  if(m_ownerThread == pthread_self())
  {
    m_ownerThread = -1;

    uint64_t count(1);
    if(write(getHandle(), &count, sizeof(count)) != sizeof(count))
      throw Exception("Failed to unlock mutex: " + lastError());
  }
  else
    throw Exception("Failed to unlock mutex: this thread is not the owner");
}

void LinuxMutex::postWaitCallback(WaitResult result)
{
  if(result == WaitSuccess)
    m_ownerThread = pthread_self();
}
