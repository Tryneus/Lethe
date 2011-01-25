#ifndef _LINUXMUTEX_H
#define _LINUXMUTEX_H

#include "LinuxSemaphore.h"
#include <stdint.h>

class LinuxMutex : private LinuxSemaphore
{
public:
   LinuxMutex(bool locked = false);
   ~LinuxMutex();

   void lock(uint32_t timeout = -1);
   void unlock();

   int getHandle();
};

#endif
