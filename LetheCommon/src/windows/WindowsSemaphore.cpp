#include "windows/WindowsSemaphore.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <Windows.h>
#include <sstream>

using namespace lethe;

const std::string WindowsSemaphore::s_baseName("Global\\lethe-semaphore-");
std::atomic<uint32_t> WindowsSemaphore::s_nextId(0);

WindowsSemaphore::WindowsSemaphore(uint32_t maxCount, uint32_t initialCount) :
  WaitObject(INVALID_HANDLE_VALUE)
{
  std::stringstream name;
  name << s_baseName << getProcessId() << "-" << s_nextId.fetch_add(1);

  m_name.assign(name.str());

  setHandle(CreateSemaphore(NULL, initialCount, maxCount, m_name.c_str()))

  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("CreateSemaphore", lastError());
}

WindowsSemaphore::WindowsSemaphore(const std::string& name)
  WaitObject(OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, false, name.c_str())),
  m_name(name)
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("CreateSemaphore", lastError());
}

WindowsSemaphore::~WindowsSemaphore()
{
  CloseHandle(getHandle());
}

void WindowsSemaphore::lock(uint32_t timeout)
{
  if(WaitForObject(*this, timeout) != WaitSuccess)
    throw std::runtime_error("failed to wait for semaphore");
}

void WindowsSemaphore::unlock(uint32_t count)
{
  if(!ReleaseSemaphore(getHandle(), count, NULL))
    throw std::bad_syscall("ReleaseSemaphore", lastError());
}
