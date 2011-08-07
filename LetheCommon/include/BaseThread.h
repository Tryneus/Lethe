#ifndef _BASETHREAD_H
#define _BASETHREAD_H

#include "WaitObject.h"
#include "LetheTypes.h"
#include "LetheBasic.h"
#include <string>
#include <queue>
#include <set>

namespace lethe
{
  // Class prototype for friending
  class CommRegistry;

  /**
   * The BaseThread class provides the common framework used by the WindowsThread
   *   and LinuxThread objects.
   *
   * BaseThread() - Sets the new thread's iteration timeout.  The new thread will
   *   iterate every time the timeout expires, unless another WaitObject triggers
   *   the thread first.  A thread can also be used as a WaitObject, which will
   *   trigger when the thread is exiting, and will stay set until the thread
   *   is destroyed.
   *
   * Public interface:
   * start() - resumes the thread's execution from the stopped state.  The thread
   *   begins stopped, and this must be called to start execution.
   * stop() - pauses the thread's execution.
   * isStopping() - returns true if the thread is in the process or has already
   *   stopped.
   * getError() - if the thread was stopped by a std::exception, getError will
   *   return the exception's what() string.
   * 
   * Protected interface, to be used by user implementation threads:
   * setup() - user-defined, called at the very beginning of the thread, from the
   *   thread's context.
   * iterate() - user-defined, called every time the iterate timeout expires, or
   *   a registered WaitObject triggers.  The handle parameter is the handle of the
   *   triggered WaitObject, or INVALID_HANDLE_VALUE if a timeout occurred.
   * abandoned() - user-defined, called every time a registered WaitObject returns
   *   an abandoned status. The handle parameter is the handle of the triggered
   *   WaitObject.
   * addWaitObject() - called to register a WaitObject with the thread.  Iterate
   *   will be called when the WaitObject triggers.
   * removeWaitObject() - called to unregister a WaitObject from the thread.
   * setWaitTimeout() - changes the iteration timeout that was set in the
   *   constructor
   */
  class BaseThread
  {
  public:
    BaseThread(uint32_t timeout);
    virtual ~BaseThread();

    void start();
    void stop();

    bool isStopping() const;
    std::string getError();

    // Act like a WaitObject without actually being one - passthrough to m_stoppedEvent
    operator WaitObject&();
    Handle getHandle() const;

  protected:
    virtual void setup();
    virtual void iterate(Handle handle);
    virtual void abandoned(Handle handle);

    friend class CommRegistry; // Workaround to allow access to addWaitObject, TODO: find a better solution

    void addWaitObject(WaitObject& obj);
    void removeWaitObject(WaitObject& obj);
    void setWaitTimeout(uint32_t timeout);

    // Internal functions to be used in WindowsThread and LinuxThread (shouldn't be used outside this library)
    void threadMain();

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    BaseThread(const BaseThread&);
    BaseThread& operator = (const BaseThread&);

    void handleObjectQueue(); // Internal function for handling queued WaitObject add/remove operations

    bool m_running; // Indicates that the thread should be looping
    bool m_exit; // Indicates that the thread is no longer startable

    Event m_triggerEvent; // An event to tell the thread to reevaluate its running state
    Event m_stoppedEvent; // An event that will be set when the thread has been stopped
    Event m_exitedEvent; // An event that will be set when the thread has exited
    Mutex m_mutex; // Mutex to limit access to the waitSet

    std::queue<std::pair<bool, WaitObject*> > m_objectQueue; // A queue of WaitObjects to add or remove
    WaitSet m_waitSet; // A list of all handles provided by the implementation along with the trigger event
    uint32_t m_timeout;

    std::string m_error; // The text of any exception that gets to the main loop
  };
}

#endif
