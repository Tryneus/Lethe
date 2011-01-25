#ifndef _LINUXHANDLESET_H
#define _LINUXHANDLESET_H

#include <sys/epoll.h>
#include <set>

#define INVALID_HANDLE_VALUE -1
#define INFINITE -1

#define WaitSuccess    0
#define WaitAbandoned -1
#define WaitTimeout   -2

class LinuxHandleSet
{
public:
  LinuxHandleSet();
  ~LinuxHandleSet();

  void add(int fd);
  void remove(int fd);

  uint32_t getSize() const;

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
