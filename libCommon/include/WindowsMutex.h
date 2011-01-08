#ifndef _WINDOWSMUTEX_H
#define _WINDOWSMUTEX_H

#include "Windows.h"

class WindowsMutex
{
public:
  WindowsMutex(bool locked = false);
  ~WindowsMutex();

  void lock();
  void unlock();

private:
  HANDLE m_handle;
};

#endif
