#ifndef _WAITOBJECT_H
#define _WAITOBJECT_H

class WaitObject;

#include "AbstractionTypes.h"
#include "AbstractionFunctions.h"

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

  // Friend WaitSets (and single-object wait), so they can access callbacks
  friend WaitResult WaitForObject(WaitObject& obj, uint32_t timeout);
  friend class WindowsWaitSet;
  friend class LinuxWaitSet;

  virtual bool preWaitCallback();
  virtual void postWaitCallback(WaitResult result);
};

#endif
