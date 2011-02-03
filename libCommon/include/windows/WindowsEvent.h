#ifndef _WINDOWSEVENT_H
#define _WINDOWSEVENT_H

#include "Windows.h"

/*
 * The WindowsEvent class provides a waitable event wrapper class for Windows.
 *  When the event is set, it will wake up any thread waiting on the handle until
 *  it is reset.
 *
 * In the future, this will be extended to add a boolean parameter to have the
 *  event automatically reset upon waking up a thread.  This requires a change to
 *  the eventfd subsystem in Linux, and a kernel module is under development for
 *  this purpose.
 */
class WindowsEvent
{
public:
  WindowsEvent(bool initialState, bool autoReset);
  ~WindowsEvent();

  HANDLE getHandle() const;

  void set();
  void reset();

private:
  HANDLE m_handle;
};

#endif
