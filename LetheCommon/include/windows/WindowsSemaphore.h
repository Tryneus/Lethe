#ifndef _WINDOWSSEMAPHORE_H
#define _WINDOWSSEMAPHORE_H

#include "WaitObject.h"
#include "LetheTypes.h"
#include "WindowsAtomic.h"

/*
 * The WindowsSemaphore class provides a wrapper of CreateSemaphore on Windows.
 */
namespace lethe
{
  class WindowsSemaphore : public WaitObject
  {
  public:
    WindowsSemaphore(uint32_t maxCount, uint32_t initialCount);
    ~WindowsSemaphore();

    void lock(uint32_t timeout = INFINITE);
    void unlock(uint32_t count);
    void error();

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsSemaphore(const WindowsSemaphore&);
    WindowsSemaphore& operator = (const WindowsSemaphore&);

    // Allow a HandleTransfer object to open an existing semaphore
    friend class WindowsHandleTransfer;
    WindowsSemaphore(const std::string& name);

    static const std::string s_semaphoreBaseName;
    static WindowsAtomic s_uniqueId;

    std::string m_name;
    bool m_error;
  };
}

#endif
