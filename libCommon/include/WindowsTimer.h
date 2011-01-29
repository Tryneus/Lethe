#ifndef _WINDOWSTIMER_H
#define _WINDOWSTIMER_H

#include "Windows.h"
#include "stdint.h"

/*
 * The WindowsTimer class wraps the CreateWaitableTimer system call. The handle
 *  remains triggered until reset.
 */
class WindowsTimer
{
public:
  WindowsTimer();
  ~WindowsTimer();

  HANDLE getHandle() const;

  void start(uint32_t timeout);
  void stop();
  void clear();

private:
  HANDLE m_handle;
  static const int64_t s_resetTimeout;
};

#endif
