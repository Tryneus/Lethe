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

void WaitObject::postWaitCallback(WaitResult result __attribute__((unused)))
{
  // Do nothing
}

