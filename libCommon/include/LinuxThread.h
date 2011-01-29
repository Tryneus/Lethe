#ifndef _LINUXTHREAD_H
#define _LINUXTHREAD_H

#include "LinuxEvent.h"
#include "LinuxHandleSet.h"
#include <set>
#include <string>
#include <stdint.h>

/*
 * The LinuxThread class is meant to be used as a base class for multi-threading.
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
 *  These file descriptors (called handles here) are the same that can be obtained from
 *  the synchronization classes Mutex, Semaphore, Event, Timer, Pipe, and even other
 *  Threads.
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
class LinuxThread
{
public:
  LinuxThread(uint32_t timeout = 0);
  virtual ~LinuxThread();
   
  void start();
  void pause();
  void stop();
   
  bool isStopping() const { return m_exit; };
  int getHandle() const { return m_exitedEvent.getHandle(); };
  const std::string& getError() const { return m_error; };
   
protected:
  virtual void iterate(int eventId) = 0;
  virtual void abandoned(int eventId);
  
  void addWaitObject(int handle);
  void removeWaitObject(int handle);
  void setWaitTimeout(uint32_t timeout);

private:
  void* threadMain();
  static void* threadHook(void*);
      
  bool m_exit; // Tells the thread to exit its main loop
   
  LinuxEvent m_runEvent; // This serves to tell the thread when it should be running
  LinuxEvent m_pauseEvent; // This serves to switch the thread to wait for a run event
  LinuxEvent m_exitedEvent; // This servers to notify anyone who waits on the thread that it has exited
   
  LinuxHandleSet m_fdSet; // A list of all handles provided by the implementation along with the pause event
  uint32_t m_timeout;
   
  pthread_t m_thread;
   
  std::string m_error; // The text of any exception that gets to the main loop
};

#endif
