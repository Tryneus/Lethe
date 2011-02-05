#ifndef _WAITOBJECT_H
#define _WAITOBJECT_H

#include "AbstractionTypes.h"

// Prototypes of HandleSets for friending purposes
class WindowsHandleSet;
class LinuxHandleSet;

class WaitObject
{
public:
  // TODO: Is a copy constructor necessary?
//  WaitObject(const WaitObject& waitObject);
  WaitObject(Handle handle);
  virtual ~WaitObject();

  Handle getHandle();
protected:
  void setWaitHandle(Handle handle);

private:
  Handle m_handle;

  // Friend HandleSets, so they can access callbacks
  friend class WindowsHandleSet;
  friend class LinuxHandleSet;

  virtual void postWaitCallback(WaitResult result);
};

bool operator < (WaitObject& left, WaitObject& right)
{
  return left.getHandle() < right.getHandle();
}

#endif
