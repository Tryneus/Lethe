#ifndef _LINUXTHREAD_H
#define _LINUXTHREAD_H

#include "LinuxEvent.h"
#include "LinuxHandleSet.h"
#include <set>
#include <string>
#include <stdint.h>

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
