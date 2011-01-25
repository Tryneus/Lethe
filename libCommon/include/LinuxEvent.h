#ifndef _LINUXEVENT_H
#define _LINUXEVENT_H

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
