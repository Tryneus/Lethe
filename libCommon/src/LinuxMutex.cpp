#include "linux/LinuxMutex.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include <pthread.h>
#include "eventfd.h"

LinuxMutex::LinuxMutex(bool locked) :
  WaitObject(eventfd((locked ? 0 : 1), (EFD_NONBLOCK | EFD_SEMAPHORE | EFD_WAITREAD))),
  m_ownerThread(locked ? pthread_self() : -1),
  m_count(locked)
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("eventfd", lastError());
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
      throw std::runtime_error("failed to wait for mutex");
  }
}

void LinuxMutex::unlock()
{
  if(m_ownerThread == pthread_self())
  {
    if(--m_count == 0)
    {
      m_ownerThread = -1;

      uint64_t buffer(1);
      if(write(getHandle(), &buffer, sizeof(buffer)) != sizeof(buffer))
        throw std::bad_syscall("write to eventfd", lastError());
    }
  }
  else
    throw std::logic_error("mutex unlocked by wrong thread");
}

bool LinuxMutex::preWaitCallback()
{
  if(m_ownerThread == pthread_self())
  {
    ++m_count;
    return true;
  }

  return false;
}

void LinuxMutex::postWaitCallback(WaitResult result)
{
  if(result == WaitSuccess)
  {
    m_ownerThread = pthread_self();
    ++m_count;
  }
}
