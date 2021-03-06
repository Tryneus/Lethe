#ifndef _WINDOWSWAITSET_H
#define _WINDOWSWAITSET_H

#include "LetheTypes.h"
#include "WaitObject.h"
#include <set>

// Prototype of the hash map, so users don't need the include
namespace mct
{
  template<typename, typename, typename, typename, typename, bool>
  class closed_hash_map;
}

/*
 * The WindowsWaitSet class provides a method of grouping and waiting on multiple
 *  handles.  Handles may be added and removed, and when a wait function is called,
 *  it will return the result of the wait as well as the handle that triggered the
 *  wakeup.  If there is an error on a handle, the wait result will be WaitAbandoned,
 *  and the user should take measures to fix or remove the broken handle.
 *
 * The underlying system call used for waiting is WaitForMultipleObjects, which
 *  has a built-in unfairness when used with the same set of Handles under high load.
 *  Because of this, handles occurring earlier in the set will be favored.  In order
 *  to combat this, the handle array is doubled and the pointer provided to
 *  WaitForMultipleObjects is moved around.  After receiving an event on a handle,
 *  the pointer is moved to just after that handle in the array, so it will be the
 *  least favored in the next call.
 */
namespace lethe
{
  class WindowsWaitSet
  {
  public:
    WindowsWaitSet();
    ~WindowsWaitSet();

    bool add(WaitObject& obj);

    bool remove(WaitObject& obj);
    bool remove(Handle handle);

    size_t getSize() const;

    WaitResult waitAny(uint32_t timeout, Handle& handle);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsWaitSet(const WindowsWaitSet&);
    WindowsWaitSet& operator = (const WindowsWaitSet&);

    void resizeEvents();
    void callPostWait(WaitResult result, Handle handle);

    static const uint32_t s_maxWaitObjects;

    mct::closed_hash_map<Handle,
                         WaitObject*,
                         std::tr1::hash<Handle>,
                         std::equal_to<Handle>,
                         std::allocator<std::pair<const Handle, WaitObject*> >,
                         false>* m_waitObjects;

    Handle* m_handleArray;
    uint32_t m_offset;
  };
}

#endif
