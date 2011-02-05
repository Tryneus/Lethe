#ifndef _WAITOBJECT_H
#define _WAITOBJECT_H

#include "AbstractionTypes.h"

// Prototypes of WaitSets for friending purposes
class WindowsWaitSet;
class LinuxWaitSet;

class WaitObject
{
public:
  WaitObject(Handle handle);
  virtual ~WaitObject();

  Handle getHandle() const;

protected:
  void setWaitHandle(Handle handle);

private:
  Handle m_handle;

  // Friend WaitSets, so they can access callbacks
  friend class WindowsWaitSet;
  friend class LinuxWaitSet;

  virtual void postWaitCallback(WaitResult result);
};

#endif
