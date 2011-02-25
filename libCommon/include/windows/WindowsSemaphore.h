#ifndef _WINDOWSSEMAPHORE_H
#define _WINDOWSSEMAPHORE_H

#include "WaitObject.h"
#include "LetheTypes.h"

/*
 * The WindowsSemaphore class provides a wrapper of CreateSemaphore on Windows.
 *  Note that there is a slight difference to how this is used on Windows vs
 *  Linux. On Windows a lock is automatically acquired on a successful wait, but
 *  on Linux, the user must explicitly call lock() to obtain the lock.
 *
 * In the future, LinuxSemaphore will be extended to automatically lock on a wait,
 *  but that will require a change to the eventfd subsystem in Linux.  A
 *  kernel module is in development to extend eventfd (see ../module).
 */
namespace lethe
{
  class WindowsSemaphore : public WaitObject
  {
  public:
    WindowsSemaphore(uint32_t maxCount, uint32_t initialCount);
    ~WindowsSemaphore();

    void lock(uint32_t timeout = INFINITE);
    void unlock(uint32_t count);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsSemaphore(const WindowsSemaphore&);
    WindowsSemaphore& operator = (const WindowsSemaphore&);
  };
}

#endif
