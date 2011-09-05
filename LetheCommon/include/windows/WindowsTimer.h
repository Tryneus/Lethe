#ifndef _WINDOWSTIMER_H
#define _WINDOWSTIMER_H

#include "WaitObject.h"
#include "LetheTypes.h"
#include "WindowsAtomic.h"
#include <string>

/*
 * The WindowsTimer class wraps the CreateWaitableTimer system call. The handle
 *  remains triggered until reset.
 */
namespace lethe
{
  class WindowsTimer : public WaitObject
  {
  public:
    explicit WindowsTimer(uint32_t timeout);
    ~WindowsTimer();

    void start(uint32_t timeout);
    void clear();
    void error();

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsTimer(const WindowsTimer&);
    WindowsTimer& operator = (const WindowsTimer&);

    // Allow a HandleTransfer object to open an existing timer
    friend class WindowsHandleTransfer;
    WindowsTimer(const std::string& name);

    static const int64_t s_resetTimeout;
    static const std::string s_timerBaseName;
    static WindowsAtomic s_uniqueId;

    std::string m_name;
    bool m_error;
  };
}

#endif
