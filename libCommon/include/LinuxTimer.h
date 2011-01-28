#ifndef _LINUXTIMER_H
#define _LINUXTIMER_H

#include <stdint.h>

/*
 * The LinuxTimer class wraps the timerfd subsystem.  When the timer expires,
 *  it remains triggered until reset.
 */
class LinuxTimer
{
public:
  LinuxTimer();
  ~LinuxTimer();
  
  int getHandle() const;
  
  void start(uint32_t timeout);
  void stop();
  void clear();

private:
  int m_fd;
};

#endif
