#include "linux/LinuxMutex.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <pthread.h>
#include "eventfd.h"

using namespace lethe;

LinuxMutex::LinuxMutex(bool locked) :
  WaitObject(eventfd((locked ? 0 : 1), (EFD_NONBLOCK | EFD_SEMAPHORE | EFD_WAITREAD))),
  m_ownerThread(locked ? pthread_self() : INVALID_THREAD_ID),
  m_lockCount(locked)
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
  if(WaitForObject(*this, timeout) != WaitSuccess) // postWaitCallback will set owner
    throw std::runtime_error("failed to wait for mutex");
}

void LinuxMutex::unlock()
{
  if(m_ownerThread == pthread_self())
  {
    if(--m_lockCount == 0)
    {
      m_ownerThread = INVALID_THREAD_ID;

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
    ++m_lockCount;
    return true;
  }

  return false;
}

void LinuxMutex::postWaitCallback(WaitResult result)
{
  if(result == WaitSuccess)
  {
    pthread_t self = pthread_self();

    if(m_ownerThread == INVALID_THREAD_ID)
    {
      m_ownerThread = self;
      ++m_lockCount;
    }
    else if(m_ownerThread != self)
      throw std::runtime_error("inconsistent mutex data"); // thread/structor test encountered this, as does functions/wait
  }
}
