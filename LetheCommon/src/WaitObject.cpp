#include "WaitObject.h"
#include "LetheInternal.h"

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

void WaitObject::setHandle(Handle handle)
{
  m_handle = handle;
}

