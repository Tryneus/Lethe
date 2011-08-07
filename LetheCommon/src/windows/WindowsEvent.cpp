#include "windows/WindowsEvent.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <Windows.h>
#include <sstream>

using namespace lethe;

const std::string WindowsEvent::s_eventBaseName("Global\\lethe-event-");
WindowsAtomic WindowsEvent::s_uniqueId(0);

WindowsEvent::WindowsEvent(bool initialState,
                           bool autoReset) :
  WaitObject(NULL)
{
  std::stringstream str;
  str << s_eventBaseName << getProcessId() << "-" << s_uniqueId.increment();
  m_name.assign(str.str());

  setWaitHandle(CreateEvent(NULL, !autoReset, initialState, m_name.c_str()));

  if(getHandle() == NULL)
    throw std::bad_syscall("CreateEvent", lastError());
}

// Constructor to be used for transferring objects between processes (by name)
WindowsEvent::WindowsEvent(const std::string& name) :
  WaitObject(NULL),
  m_name(name)
{
  setWaitHandle(OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, false, m_name.c_str()));

  if(getHandle() == NULL)
    throw std::bad_syscall("CreateEvent", lastError());
}

// Constructor to be used to specify the name during creation, used by WindowsPipe
  WindowsEvent::WindowsEvent(bool initialState,
                             bool autoReset,
                             const std::string& name) :
  WaitObject(NULL),
  m_name(s_eventBaseName + name)
{
  setWaitHandle(OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, false, m_name.c_str()));

  if(getHandle() == NULL)
  {
    setWaitHandle(CreateEvent(NULL, !autoReset, initialState, m_name.c_str()));

    if(getHandle() == NULL)
      throw std::bad_syscall("CreateEvent", lastError());
  }
}

WindowsEvent::~WindowsEvent()
{
  CloseHandle(getHandle());
}

void WindowsEvent::set()
{
  if(!SetEvent(getHandle()))
    throw std::bad_syscall("SetEvent", lastError());
}

void WindowsEvent::reset()
{
  if(!ResetEvent(getHandle()))
    throw std::bad_syscall("ResetEVent", lastError());
}
