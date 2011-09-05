#ifndef _WINDOWSMUTEX_H
#define _WINDOWSMUTEX_H

#include "WaitObject.h"
#include "LetheTypes.h"
#include "WindowsAtomic.h"
#include <string>

/*
 * The WindowsMutex class is a wrapper class of CreateMutex on Windows.
 */
namespace lethe
{
  class WindowsMutex : public WaitObject
  {
  public:
    explicit WindowsMutex(bool locked);
    ~WindowsMutex();

    void lock(uint32_t timeout = INFINITE);
    void unlock();
    void error();

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsMutex(const WindowsMutex&);
    WindowsMutex& operator = (const WindowsMutex&);

    // Allow a HandleTransfer object to open an existing mutex
    friend class WindowsHandleTransfer;
    WindowsMutex(const std::string& name);

    // Allow a pipe to create a new event with a specific name
    friend class WindowsPipe;
    WindowsMutex(bool locked, const std::string& name);

    static const std::string s_mutexBaseName;
    static WindowsAtomic s_uniqueId;

    std::string m_name;
    bool m_error;
  };
}

#endif
