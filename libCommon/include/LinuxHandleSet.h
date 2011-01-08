#ifndef _LINUXHANDLESET_H
#define _LINUXHANDLESET_H

#include <sys/epoll.h>
#include <set>

#define INVALID_HANDLE_VALUE -1
#define INFINITE -1

#define WaitSuccess    0
#define WaitError     -1
#define WaitAbandoned -2
#define WaitTimeout   -3

class LinuxHandleSet
{
public:
   LinuxHandleSet();
   ~LinuxHandleSet();

   bool add(int fd);
   bool remove(int fd);

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
