#ifndef _WINDOWSTHREAD_H
#define _WINDOWSTHREAD_H

#include "LetheTypes.h"
#include "BaseThread.h"
#include "WindowsEvent.h"
#include "WindowsWaitSet.h"
#include <Windows.h>
#include <string>
#include <set>

/*
 * The WindowsThread class is meant to be used as a base class for multi-threading.
 *  The user should derive from this and implement the iterate (and possibly
 *  abandon) functions.
 *
 * iterate() is called every time the thread loops.  The thread only begins looping
 *  after start() has been called (this may be done in the derived class's constructor
 *  to always begin immediately).  If pause() is called, the thread may be restarted
 *  with start().  At any point, the thread may be closed with stop(), and which point
 *  it cannot be restarted.
 *
 * If the thread is interrupted by an unhandled exception or encounters an internal
 *  error, the error string may be obtained through getError().
 *
 * Thread loop rate may be controlled by a few factors.  First, a timeout may be set
 *  through the constructor, which gives the base time between loops (in ms).  Secondly,
 *  the thread may provide wait objects to wake the thread up if one becomes triggered.
 *  These Handles are the same that can be obtained from the synchronization classes
 *  Mutex, Semaphore, Event, Timer, Pipe, and even other Threads.
 *
 * The thread handle triggers when the thread exits and does not reset.
 *
 * If the thread is destroyed while running, it will stop the thread and wait for it to
 *  exit before returning from the destructor.
 *
 * error (optionally implemented in derived classes) is called when there is a
 *  problem with a user-provided handle.  Thread will not automatically remove this handle,
 *  but the user needs to fix or remove it, or error will keep getting called.
 */
namespace lethe
{
  class WindowsThread : public BaseThread
  {
  public:
    explicit WindowsThread(uint32_t timeout);
    virtual ~WindowsThread();

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsThread(const WindowsThread&);
    WindowsThread& operator = (const WindowsThread&);

    static DWORD WINAPI threadHook(void*);
    Handle m_handle;
  };
}

#endif
