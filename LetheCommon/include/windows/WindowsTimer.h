#ifndef _WINDOWSTIMER_H
#define _WINDOWSTIMER_H

#include "WaitObject.h"
#include "LetheTypes.h"

/*
 * The WindowsTimer class wraps the CreateWaitableTimer system call. The handle
 *  remains triggered until reset.
 */
namespace lethe
{
  class WindowsTimer : public WaitObject
  {
  public:
    WindowsTimer();
    ~WindowsTimer();

    void start(uint32_t timeout);
    void clear();

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsTimer(const WindowsTimer&);
    WindowsTimer& operator = (const WindowsTimer&);

    static const int64_t s_resetTimeout;
  };
}

#endif
