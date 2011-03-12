#ifndef _LINUXWAITSET_H
#define _LINUXWAITSET_H

#include "LetheTypes.h"
#include "WaitObject.h"
#include <tr1/functional>
#include <poll.h>
#include <list>
#include <set>

// Prototype of the hash map, so users don't need the include
namespace mct
{
  template<typename, typename, typename, typename, typename, bool>
  class closed_hash_map;
}

/*
 * The LinuxWaitSet class provides a method of grouping and waiting on multiple
 *  handles (file descriptors in Linux).  Handles may be added and removed, and when
 *  a wait function is called, it will return the result of the wait as well as the
 *  handle that triggered the wakeup.  If there is an error on a handle, the wait
 *  result will be WaitAbandoned, and the user should take measures to fix or remove
 *  the broken handle.
 *
 * The underlying system call used for waiting is poll, which allows minimal
 *  overhead per wait call.  A single call may return multiple wait objects.  This
 *  may lead to a deadlock if used incorrectly.
 */
namespace lethe
{
  class LinuxWaitSet
  {
  public:
    LinuxWaitSet();
    ~LinuxWaitSet();

    bool add(WaitObject& obj);

    bool remove(WaitObject& obj);
    bool remove(Handle handle);

    size_t getSize() const;

    WaitResult waitAny(uint32_t timeout, Handle& handle);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxWaitSet(const LinuxWaitSet&);
    LinuxWaitSet& operator = (const LinuxWaitSet&);

    void resizeEvents();
    void findBadHandles();
    void addEvents(const std::list<Handle>& events);
    WaitResult pollEvents(uint32_t timeout, uint32_t endTime);
    WaitResult getEvent(Handle& handle);

    mct::closed_hash_map<Handle,
                         WaitObject*,
                         std::tr1::hash<Handle>,
                         std::equal_to<Handle>,
                         std::allocator<std::pair<const Handle, WaitObject*> >,
                         false>* m_waitObjects;

    pollfd* m_waitArray;
    uint32_t m_eventOffset;
  };
}

#endif
