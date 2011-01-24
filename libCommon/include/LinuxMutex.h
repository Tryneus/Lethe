#ifndef _LINUXMUTEX_H
#define _LINUXMUTEX_H

#include <pthread.h>

class LinuxMutex
{
public:
   LinuxMutex(bool locked = false);
   ~LinuxMutex();

   void lock(int timeout = -1);
   void unlock();

private:
//   pthread_t m_lockingThread;
//   int m_pipeRead;
//   int m_pipeWrite;
};

#endif
