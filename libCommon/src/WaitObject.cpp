#include "WaitObject.h"

WaitObject(const WaitObject& source) :
  m_handle(source.handle)
{
  // Do nothing
}

WaitObject(Handle handle) :
  m_handle(handle)
{
  // Do nothing
}

virtual ~WaitObject()
{
  // Do nothing
}

Handle getHandle()
{
  return m_handle;
}

void setWaitHandle(Handle handle)
{
  m_handle = handle;
}

virtual void postWaitCallback(WaitResult result)
{
  // Do nothing
}


#endif
