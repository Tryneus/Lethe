#ifndef _WINDOWSEVENT_H
#define _WINDOWSEVENT_H

#include "WaitObject.h"

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
namespace lethe
{
  class WindowsEvent : public WaitObject
  {
  public:
    WindowsEvent(bool initialState, bool autoReset);
    ~WindowsEvent();

    void set();
    void reset();

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsEvent(const WindowsEvent&);
    WindowsEvent& operator = (const WindowsEvent&);
  };
}

#endif
