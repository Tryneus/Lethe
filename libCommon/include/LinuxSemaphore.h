#ifndef _LINUXSEMAPHORE_H
#define _LINUXSEMAPHORE_H

#include <stdint.h>

class LinuxSemaphore
{
public:
  LinuxSemaphore(uint32_t maxCount, uint32_t initialCount);
  ~LinuxSemaphore();

  void lock(uint32_t timeout = -1);
  void unlock(uint32_t count);

  int getHandle();

private:
  int m_semaphore;
};

#endif
