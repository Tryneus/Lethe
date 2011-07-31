#ifndef _LINUXMUTEX_H
#define _LINUXMUTEX_H

#include "WaitObject.h"
#include "LetheTypes.h"
#include <pthread.h>

/*
 * The LinuxMutex class is extremely similar to LinuxSemaphore, being a
 *  semaphore with a maximum value of 1.
 */
namespace lethe
{
  class LinuxMutex : public WaitObject
  {
  public:
    LinuxMutex(bool locked = false);
    ~LinuxMutex();

    void lock(uint32_t timeout = INFINITE);
    void unlock();

  protected:
    bool preWaitCallback();
    void postWaitCallback(WaitResult result);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxMutex(const LinuxMutex&);
    LinuxMutex& operator = (const LinuxMutex&);

    static const pthread_t INVALID_THREAD_ID = -1;

    pthread_t m_ownerThread;
    uint32_t m_lockCount;
  };
}

#endif
