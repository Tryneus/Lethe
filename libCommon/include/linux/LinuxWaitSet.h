#ifndef _LINUXWAITSET_H
#define _LINUXWAITSET_H

#include "AbstractionTypes.h"
#include "WaitObject.h"
#include <tr1/functional>
#include <sys/epoll.h>
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
 * The underlying system call used for waiting is epoll_wait, which allows minimal
 *  overhead per wait call.  An epoll set is configured with the file descriptors
 *  being waited on, and a single call may return multiple events (allowing fair
 *  attention to multiple busy interfaces).
 *
 * The waitAll function is currently unimplemented on Linux.  There are no plans as
 *  to how to implement this at the moment.
 */
class LinuxWaitSet
{
public:
  LinuxWaitSet();
  ~LinuxWaitSet();

  void add(WaitObject& obj);

  void remove(WaitObject& obj);
  void remove(Handle handle);

  size_t getSize() const;

  WaitResult waitAll(uint32_t timeout, Handle& handle);
  WaitResult waitAny(uint32_t timeout, Handle& handle);

private:
  void resizeEvents();

  Handle m_epollSet;

  mct::closed_hash_map<Handle,
                       WaitObject*,
                       std::tr1::hash<Handle>,
                       std::equal_to<Handle>,
                       std::allocator<std::pair<const Handle, WaitObject*> >,
                       false>* m_waitObjects;

  epoll_event* m_events;
  int m_eventCount;
};

#endif
