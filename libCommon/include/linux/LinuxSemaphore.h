#ifndef _LINUXSEMAPHORE_H
#define _LINUXSEMAPHORE_H

#include "WaitObject.h"
#include "AbstractionTypes.h"

/*
 * The LinuxSemaphore class provides a wrapper to the eventfd subsystem,
 *  using semaphore mode. Once a wait has been completed on the Semaphore
 *  handle, the user must call lock() to obtain the lock.
 *
 * In the future, this will be extended to automatically lock on a wait,
 *  but that will require a change to the eventfd subsystem in Linux.  A
 *  kernel module is in development to extend eventfd (see ../module).
 */
class LinuxSemaphore : public WaitObject
{
public:
  LinuxSemaphore(uint32_t maxCount, uint32_t initialCount);
  ~LinuxSemaphore();

  void lock(uint32_t timeout = INFINITE);
  void unlock(uint32_t count);
};

#endif
