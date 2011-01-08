#ifndef _WINDOWSEVENT_H
#define _WINDOWSEVENT_H

#include "Windows.h"

class WindowsEvent
{
public:
  WindowsEvent(bool initialState);
  ~WindowsEvent();

  HANDLE getHandle() const;

  void set();
  void reset();

private:
  HANDLE m_handle;
};

#endif
