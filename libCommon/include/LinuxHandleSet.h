#ifndef _LINUXHANDLESET_H
#define _LINUXHANDLESET_H

#include <sys/epoll.h>
#include <set>

// Some defines to copy identifiers used in WIN32
#define INVALID_HANDLE_VALUE -1
#define INFINITE -1

#define WaitSuccess    0
#define WaitAbandoned -1
#define WaitTimeout   -2

/*
 * The LinuxHandleSet class provides a method of grouping and waiting on multiple
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
class LinuxHandleSet
{
public:
  LinuxHandleSet();
  ~LinuxHandleSet();

  void add(int fd);
  void remove(int fd);

  size_t getSize() const;
  const std::set<int>& getSet() const;

  int waitAll(uint32_t timeout, int& fd);
  int waitAny(uint32_t timeout, int& fd);

private:
  void resizeEvents();

  int m_epollSet;
  std::set<int> m_fdSet;

  epoll_event* m_events;
  int m_eventCount;
};

#endif
