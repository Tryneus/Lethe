#ifndef _WINDOWSEVENT_H
#define _WINDOWSEVENT_H

#include "WaitObject.h"
#include "WindowsAtomic.h"
#include <string>

/*
 * The WindowsEvent class provides a waitable event wrapper class for Windows.
 *  When the event is set, it will wake up any thread waiting on the handle until
 *  it is reset.
 */
namespace lethe
{
  class WindowsEvent : public WaitObject
  {
  public:
    WindowsEvent(bool initialState, bool autoReset);
    ~WindowsEvent();

    void set();
    void reset();

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsEvent(const WindowsEvent&);
    WindowsEvent& operator = (const WindowsEvent&);

    // Allow a HandleTransfer object to open an existing event
    friend class WindowsHandleTransfer;
    WindowsEvent(const std::string& name);

    // Allow a pipe to create a new event with a specific name
    friend class WindowsPipe;
    WindowsEvent(bool initialState, bool autoReset, const std::string& name);

    static const std::string s_eventBaseName;
    static WindowsAtomic s_uniqueId;

    std::string m_name;
  };
}

#endif
