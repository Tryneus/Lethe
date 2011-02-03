#ifndef _LINUXMUTEX_H
#define _LINUXMUTEX_H

#include "LinuxSemaphore.h"
#include <pthread.h>
#include <stdint.h>

/*
 * The LinuxMutex class is a specialization of LinuxSemaphore, being a
 *  semaphore with a maximum value of 1.  Once a wait has been completed
 *  on the Mutex handle, the user must call lock() to obtain the lock.
 *
 * In the future, this will be extended to automatically lock on a wait,
 *  but that will require a change to the eventfd subsystem in Linux.  A
 *  kernel module is in development to extend eventfd (see ../module).
 */
class LinuxMutex : private LinuxSemaphore
{
public:
   LinuxMutex(bool locked = false);
   ~LinuxMutex();

   void lock(uint32_t timeout = -1);
   void unlock();

   int getHandle();

private:
   pthread_t m_ownerThread;
};

#endif
