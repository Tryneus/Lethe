#ifndef _WINDOWSMUTEX_H
#define _WINDOWSMUTEX_H

#include "Windows.h"

class WindowsMutex
{
public:
  WindowsMutex(bool locked = false);
  ~WindowsMutex();

  void lock(uint32_t timeout = -1);
  void unlock();

  HANDLE getHandle();

private:
  HANDLE m_handle;
};

#endif
