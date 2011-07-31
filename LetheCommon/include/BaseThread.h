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
