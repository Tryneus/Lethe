#ifndef _WINDOWSMUTEX_H
#define _WINDOWSMUTEX_H

#include "WaitObject.h"
#include "LetheTypes.h"

/*
 * The WindowsMutex class is a wrapper class of CreateMutex on Windows.  Note that
 *  there is a slight difference in how these work on Linux.  In Windows a lock is
 *  acquired on a successful wait, but on Linux lock() must be explicitly called.
 */
namespace lethe
{
  class WindowsMutex : public WaitObject
  {
  public:
    WindowsMutex(bool locked = false);
    ~WindowsMutex();

    void lock(uint32_t timeout = INFINITE);
    void unlock();

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsMutex(const WindowsMutex&);
    WindowsMutex& operator = (const WindowsMutex&);
  };
}

#endif
