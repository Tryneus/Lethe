#ifndef _WINDOWSSEMAPHORE_H
#define _WINDOWSSEMAPHORE_H

#include "Windows.h"
#include "stdint.h"

class WindowsSemaphore
{
public:
  WindowsSemaphore(uint32_t maxCount, uint32_t initialCount);
  ~WindowsSemaphore();

  void lock(uint32_t timeout = -1);
  void unlock(uint32_t count);

  HANDLE getHandle();

private:
  HANDLE m_semaphore;
};

#endif
