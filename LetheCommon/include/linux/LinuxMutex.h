#ifndef _LINUXMUTEX_H
#define _LINUXMUTEX_H

#include "WaitObject.h"
#include "LetheTypes.h"

/*
 * The LinuxMutex class is extremely similar to LinuxSemaphore, being a
 *  semaphore with a maximum value of 1.
 */
namespace lethe
{
  class LinuxMutex : public WaitObject
  {
  public:
    explicit LinuxMutex(bool locked);
    ~LinuxMutex();

    void lock(uint32_t timeout = INFINITE);
    void unlock();
    void error();

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxMutex(const LinuxMutex&);
    LinuxMutex& operator = (const LinuxMutex&);

    // Allow LinuxSemaphore to be constructed by a handle transfer from another process
    friend class LinuxHandleTransfer;
    LinuxMutex(Handle handle);

    static const std::string s_eventfdDevice;
  };
}

#endif
