#include "WaitObject.h"

WaitObject::WaitObject(Handle handle) :
  m_handle(handle)
{
  // Do nothing
}

WaitObject::~WaitObject()
{
  // Do nothing
}

Handle WaitObject::getHandle() const
{
  return m_handle;
}

void WaitObject::setWaitHandle(Handle handle)
{
  m_handle = handle;
}

#if defined(__GNUG__) /* Suppress warning in GCC */
void WaitObject::postWaitCallback(WaitResult result __attribute__((unused)))
#else
void WaitObject::postWaitCallback(WaitResult result)
#endif
{
  // Do nothing
}

