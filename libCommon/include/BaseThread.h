#ifndef _BASETHREAD_H
#define _BASETHREAD_H

#include <set>
#include <string>
#include "AbstractionBasic.h"

class BaseThread
{
public:
  BaseThread(uint32_t timeout);
  virtual ~BaseThread();

  void start();
  void pause();
  void stop();

  bool isStopping() const;
  Handle getHandle() const;
  const std::string& getError() const;

protected:
  virtual void iterate(Handle handle) = 0;
  virtual void abandoned(Handle handle);

  void addWaitObject(Handle handle);
  void removeWaitObject(Handle handle);
  void setWaitTimeout(uint32_t timeout);

  // Internal functions to be used in WindowsThread and LinuxThread (not to be exported for users)
  void* threadMain();

private:
  bool m_exit; // Tells the thread to exit its main loop

  Event m_runEvent; // This serves to tell the thread when it should be running
  Event m_pauseEvent; // This serves to switch the thread to wait for a run event
  Event m_exitedEvent; // An event that will be triggered when the thread exits

  HandleSet m_handleSet; // A list of all handles provided by the implementation along with the pause event
  uint32_t m_timeout;

  std::string m_error; // The text of any exception that gets to the main loop
};

#endif
