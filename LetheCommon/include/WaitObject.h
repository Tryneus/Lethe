#ifndef _WAITOBJECT_H
#define _WAITOBJECT_H

#include "LetheTypes.h"
#include "LetheFunctions.h"

namespace lethe
{
  // Prototypes of WaitSets for friending purposes
  class WindowsWaitSet;
  class LinuxWaitSet;

  /**
   * The WaitObject class provides the framework for cross-thread and cross-
   *   process synchronization objects to be used by WaitSets and the
   *   WaitForObject function.
   *
   * getHandle() - returns the handle corresponding to the wait object.  This
   *   is the value used with the operating system for synchronization calls.
   *   On Windows, this is a HANDLE (void*), and on Linux, this is a file
   *   descriptor (int).
   *
   * setHandle() - used if the Handle of the WaitObject is not known at
   *   construction of the base class, changes the handle that will be used.
   *   To avoid problems, this should never be used after the derived object
   *   has finished construction.
   *
   */
  class WaitObject
  {
  public:
    WaitObject(Handle handle);
    virtual ~WaitObject();

    Handle getHandle() const;

  protected:
    void setHandle(Handle handle);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WaitObject(const WaitObject&);
    WaitObject& operator = (const WaitObject&);

    Handle m_handle;
  };
}

#endif
