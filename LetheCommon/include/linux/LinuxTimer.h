#ifndef _LINUXTIMER_H
#define _LINUXTIMER_H

#include "WaitObject.h"
#include "LetheTypes.h"

/*
 * The LinuxTimer class wraps the timerfd subsystem.  When the timer expires,
 *  it remains triggered until reset.
 */
namespace lethe
{
  // Prototype for transferring handles between processes - defined in libProcessComm
  class LinuxHandleTransfer;

  class LinuxTimer : public WaitObject
  {
  public:
    LinuxTimer(uint32_t timeout, bool periodic, bool autoReset);
    ~LinuxTimer();

    void start(uint32_t timeout, bool periodic);
    void clear();
    void error();

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxTimer(const LinuxTimer&);
    LinuxTimer& operator = (const LinuxTimer&);

    // Allow LinuxTimer to be constructed by a handle transfer from another process
    friend class LinuxHandleTransfer;
    LinuxTimer(Handle handle);

    static const std::string s_timerfdDevice;
  };
}

#endif
