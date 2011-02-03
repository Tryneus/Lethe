#include "linux/LinuxMutex.h"
#include "Exception.h"
#include <pthread.h>

LinuxMutex::LinuxMutex(bool locked) :
  LinuxSemaphore(1, (locked ? 0 : 1)),
  m_ownerThread(locked ? pthread_self() : -1)
{
  // Do nothing
}

LinuxMutex::~LinuxMutex()
{
  // Do nothing
}

void LinuxMutex::lock(uint32_t timeout)
{
  if(m_ownerThread != pthread_self())
  {
    LinuxSemaphore::lock(timeout);
    m_ownerThread = pthread_self();
  }
}

void LinuxMutex::unlock()
{
  if(m_ownerThread == (pthread_t)(-1) || m_ownerThread == pthread_self())
  {
    LinuxSemaphore::unlock(1);
    m_ownerThread = -1;
  }
  else
    // TODO: this exception will only be thrown if the mutex is explicitly
    //  locked() by a different thread.  A waitlock will not keep track of the
    //  thread id.
    throw Exception("Cannot unlock mutex from a different thread");
}

int LinuxMutex::getHandle()
{
  return LinuxSemaphore::getHandle();
}

