#ifndef _WINDOWSTIMER_H
#define _WINDOWSTIMER_H

#include "WaitObject.h"
#include "AbstractionTypes.h"

/*
 * The WindowsTimer class wraps the CreateWaitableTimer system call. The handle
 *  remains triggered until reset.
 */
class WindowsTimer : public WaitObject
{
public:
  WindowsTimer();
  ~WindowsTimer();

  void start(uint32_t timeout);
  void clear();

private:
  static const int64_t s_resetTimeout;
};

#endif
