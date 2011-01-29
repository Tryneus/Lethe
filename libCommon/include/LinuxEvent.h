#ifndef _LINUXEVENT_H
#define _LINUXEVENT_H

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
class LinuxEvent
{
public:
  LinuxEvent(bool initialState);
  ~LinuxEvent();
   
  int getHandle() const;
   
  void set();
  void reset();
   
private:
  int m_event;
};

#endif
