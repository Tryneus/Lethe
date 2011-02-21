#ifndef _LINUXTIMER_H
#define _LINUXTIMER_H

#include "WaitObject.h"
#include "AbstractionTypes.h"

/*
 * The LinuxTimer class wraps the timerfd subsystem.  When the timer expires,
 *  it remains triggered until reset.
 */
class LinuxTimer : public WaitObject
{
public:
  LinuxTimer();
  ~LinuxTimer();

  void start(uint32_t timeout);
  void clear();

private:
  // Private, undefined copy constructor and assignment operator so they can't be used
  LinuxTimer(const LinuxTimer&);
  LinuxTimer& operator = (const LinuxTimer&);
};

#endif
