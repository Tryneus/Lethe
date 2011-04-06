#ifndef _LINUXEVENT_H
#define _LINUXEVENT_H

#include "WaitObject.h"

/*
 * The LinuxEvent class provides a waitable event wrapper class for Linux.
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
  // Prototype for transferring handles between processes - defined in libProcessComm
  class LinuxHandleTransfer;

  class LinuxEvent : public WaitObject
  {
  public:
    LinuxEvent(bool initialState, bool autoReset);
    ~LinuxEvent();

    void set();
    void reset();

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxEvent(const LinuxEvent&);
    LinuxEvent& operator = (const LinuxEvent&);

    // Allow LinuxEvent to be constructed by a handle transfer from another process
    friend class LinuxHandleTransfer;
    LinuxEvent(Handle handle);
  };
}

#endif
