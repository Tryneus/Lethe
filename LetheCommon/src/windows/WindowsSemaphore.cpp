#include "windows/WindowsSemaphore.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <Windows.h>
#include <sstream>

using namespace lethe;

const std::string WindowsSemaphore::s_semaphoreBaseName("Global\\lethe-semaphore-");
WindowsAtomic WindowsSemaphore::s_uniqueId(0);

WindowsSemaphore::WindowsSemaphore(uint32_t maxCount, uint32_t initialCount) :
  WaitObject(INVALID_HANDLE_VALUE)
{
  std::stringstream str;
  str << s_semaphoreBaseName << getProcessId() << "-" << s_uniqueId.increment();
  m_name.assign(str.str());

  setWaitHandle(CreateSemaphore(NULL, initialCount, maxCount, m_name.c_str()));

  if(getHandle() == NULL)
    throw std::bad_syscall("CreateSemaphore", lastError());
}

WindowsSemaphore::WindowsSemaphore(const std::string& name) : 
  WaitObject(NULL),
  m_name(name)
{
  setWaitHandle(OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, false, name.c_str()));

  if(getHandle() == NULL)
    throw std::bad_syscall("CreateSemaphore", lastError());
}

WindowsSemaphore::~WindowsSemaphore()
{
  CloseHandle(getHandle());
}

void WindowsSemaphore::lock(uint32_t timeout)
{
  if(WaitForObject(getHandle(), timeout) != WaitSuccess)
    throw std::runtime_error("failed to wait for semaphore");
}

void WindowsSemaphore::unlock(uint32_t count)
{
  if(!ReleaseSemaphore(getHandle(), count, NULL))
    throw std::bad_syscall("ReleaseSemaphore", lastError());
}

void WindowsSemaphore::error()
{
}
