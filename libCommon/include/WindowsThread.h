#ifndef _WINDOWSTHREAD_H
#define _WINDOWSTHREAD_H

#include <set>
#include <string>
#include "WindowsEvent.h"
#include "WindowsHandleSet.h"
#include "Windows.h"
#include "stdint.h"

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
 * abandoned (optionally implemented in derived classes) is called when there is a
 *  problem with a user-provided handle.  Thread will not automatically remove this handle,
 *  but the user needs to fix or remove it, or abandoned will keep getting called.
 */
class WindowsThread
{
public:
  WindowsThread(uint32_t timeout = 0);
  virtual ~WindowsThread();

  void start();
  void pause();
  void stop();

  bool isStopping() const { return m_exit; };
  HANDLE getHandle() const { return m_handle; };
  const std::string& getError() const { return m_error; };

protected:
  virtual void iterate(HANDLE handle) = 0;
  virtual void abandoned(HANDLE handle);

  void addWaitObject(HANDLE handle);
  void removeWaitObject(HANDLE handle);
  void setWaitTimeout(uint32_t timeout);

private:
  DWORD threadMain();

  static DWORD WINAPI threadHook(void*);

  bool m_exit; // Tells the thread to exit its main loop

  WindowsEvent m_runEvent; // This serves to tell the thread when it should be running
  WindowsEvent m_pauseEvent; // This serves to switch the thread to wait for a run event

  WindowsHandleSet m_handleSet; // A list of all handles provided by the implementation along with the pause event
  uint32_t m_timeout;

  HANDLE m_handle; // The WIN32 handle of the thread
  std::string m_error; // The text of any exception that gets to the main loop
};

#endif
