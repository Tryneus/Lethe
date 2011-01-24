#ifndef _LINUXSEMAPHORE_H
#define _LINUXSEMAPHORE_H

#include <sys/eventfd.h>

class WindowsSemaphore
{
public:
  LinuxSemaphore(uint32_t maxCount, uint32_t initialCount);
  ~LinuxSemaphore();

  void lock();
  void unlock(uint32_t count);

  int getHandle();

private:
  int m_semaphore;
}

#endif