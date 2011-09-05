#include "windows/WindowsMutex.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <Windows.h>
#include <sstream>

using namespace lethe;

const std::string WindowsMutex::s_mutexBaseName("Global\\lethe-mutex-");
WindowsAtomic WindowsMutex::s_uniqueId(0);

WindowsMutex::WindowsMutex(bool locked) :
  WaitObject(NULL)
{
  std::stringstream str;
  str << s_mutexBaseName << getProcessId() << "-" << s_uniqueId.increment();
  m_name.assign(str.str().c_str());

  setWaitHandle(CreateMutex(NULL, locked, m_name.c_str()));

  if(getHandle() == NULL)
    throw std::bad_syscall("CreateMutex", lastError());
}

// Constructor to be used for transferring objects between processes (by name)
WindowsMutex::WindowsMutex(const std::string& name) :
  WaitObject(NULL),
  m_name(name)
{
  setWaitHandle(OpenMutex(MUTEX_MODIFY_STATE | SYNCHRONIZE, false, m_name.c_str()));

  if(getHandle() == NULL)
    throw std::bad_syscall("CreateMutex", lastError());
}

// Constructor to be used to specify the name during creation, used by WindowsPipe
  WindowsMutex::WindowsMutex(bool locked, const std::string& name) :
  WaitObject(NULL),
  m_name(s_mutexBaseName + name)
{
  // First try to open it, then create it
  setWaitHandle(OpenMutex(MUTEX_MODIFY_STATE | SYNCHRONIZE, false, m_name.c_str()));

  if(getHandle() == NULL)
  {
    setWaitHandle(CreateMutex(NULL, locked, m_name.c_str()));

    if(getHandle() == NULL)
      throw std::bad_syscall("CreateMutex", lastError());
  }
}

WindowsMutex::~WindowsMutex()
{
  CloseHandle(getHandle());
}

void WindowsMutex::lock(uint32_t timeout)
{
  if(WaitForObject(getHandle(), timeout) != WaitSuccess)
    throw std::runtime_error("failed to wait for mutex");
}

void WindowsMutex::unlock()
{
  if(!ReleaseMutex(getHandle()))
    throw std::bad_syscall("ReleaseMutex", lastError());
}
