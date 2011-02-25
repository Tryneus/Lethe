#include "WaitObject.h"

using namespace lethe;

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

bool WaitObject::preWaitCallback()
{
  return false;
}

void WaitObject::postWaitCallback(WaitResult result GCC_UNUSED)
{
  // Do nothing
}

