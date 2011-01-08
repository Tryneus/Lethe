#ifndef _WINDOWSTHREAD_H
#define _WINDOWSTHREAD_H

#include <set>
#include <string>
#include "WindowsEvent.h"
#include "WindowsHandleSet.h"
#include "Windows.h"
#include "stdint.h"

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

  bool addWaitObject(HANDLE handle);
  bool removeWaitObject(HANDLE handle);
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
