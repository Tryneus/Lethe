#include "linux/LinuxSemaphore.h"
#include "LetheTypes.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include "eventfd.h"
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace lethe;

LinuxSemaphore::LinuxSemaphore(uint32_t maxCount,
                               uint32_t initialCount) :
  WaitObject(eventfd(initialCount, (EFD_NONBLOCK | EFD_SEMAPHORE | EFD_WAITREAD))),
  m_maxCount(maxCount),
  m_count(initialCount)
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("eventfd", lastError());
}

LinuxSemaphore::LinuxSemaphore(Handle handle) :
  WaitObject(handle),
  m_maxCount(0xFFFFFFFF),
  m_count(0)
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::invalid_argument("handle");

  struct stat handleInfo;
  if(fstat(handle, &handleInfo) != 0) // TODO: check if handle is for an eventfd
  {
    close(handle);
    throw std::bad_syscall("fstat", lastError());
  }
}

LinuxSemaphore::~LinuxSemaphore()
{
  close(getHandle());
}

const std::string& LinuxSemaphore::name() const
{
  return m_name;
}

void LinuxSemaphore::lock(uint32_t timeout)
{
  if(WaitForObject(*this, timeout) != WaitSuccess)
    throw std::runtime_error("failed to wait for semaphore");
}

void LinuxSemaphore::unlock(uint32_t count)
{
  if(m_count.fetch_add(count) + count > m_maxCount)
  {
    m_count.fetch_sub(count);
    throw std::bad_syscall("write", "semaphore full");
  }

  uint64_t internalCount(count);

  if(write(getHandle(), &internalCount, sizeof(internalCount)) != sizeof(internalCount))
    throw std::bad_syscall("write to eventfd", lastError());
}

void LinuxSemaphore::postWaitCallback(WaitResult result)
{
  if(result == WaitSuccess)
    m_count.fetch_sub(1);
}
