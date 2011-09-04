#include "linux/LinuxPipe.h"
#include "LetheTypes.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include "LetheInternal.h"
#include <sstream>
#include <queue>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <aio.h>

using namespace lethe;

const std::string LinuxPipe::s_fifoPath("/tmp/lethe/");
const std::string LinuxPipe::s_fifoBaseName("lethe-fifo-");
LinuxAtomic LinuxPipe::s_uniqueId(0);

LinuxPipe::LinuxPipe() :
  m_waitObject(NULL),
  m_pipeRead(INVALID_HANDLE_VALUE),
  m_pipeWrite(INVALID_HANDLE_VALUE),
  m_asyncStart(0),
  m_asyncEnd(0),
  m_blockingWrite(false),
  m_inCreated(false),
  m_outCreated(false)
{
  // Make sure the fifo path exists
  if(mkdir(s_fifoPath.c_str(), 0777) != 0 && errno != EEXIST)
    throw std::bad_syscall("mkdir", lastError());

  // No name provided, auto-generate one
  std::stringstream str;
  str << s_fifoPath << s_fifoBaseName << getProcessId() << "-" << s_uniqueId.increment();
  m_fifoReadName.assign(str.str());
  m_fifoWriteName.assign(str.str());

  try
  {
    // This is a one-way pipe, so both sides open the same file
    if(mkfifo(m_fifoReadName.c_str(), 0777) != 0 && errno != EEXIST) // TODO: permissions?
      throw std::bad_syscall("mkfifo", lastError());

    m_inCreated = true;
    m_outCreated = true;

    m_pipeRead = open(m_fifoReadName.c_str(), O_RDWR | O_NONBLOCK);
    if(m_pipeRead == INVALID_HANDLE_VALUE)
      throw std::bad_syscall("open", lastError() + ", " + m_fifoReadName);

    if(!setCloseOnExec(m_pipeRead))
      throw std::bad_syscall("fcntl", lastError());

    m_pipeWrite = m_pipeRead;
    m_waitObject = new WaitObject(m_pipeRead);
  }
  catch(...)
  {
    cleanup();
    throw;
  }

  // Prepare async event structures
  memset(m_asyncArray, 0, sizeof(m_asyncArray));
  for(uint32_t i = 0; i < s_maxAsyncEvents; ++i)
  {
    m_asyncArray[i].aio_fildes = m_pipeWrite;
  }
}

LinuxPipe::LinuxPipe(const std::string& pipeIn, bool createIn, const std::string& pipeOut, bool createOut) :
  m_waitObject(NULL),
  m_pipeRead(INVALID_HANDLE_VALUE),
  m_pipeWrite(INVALID_HANDLE_VALUE),
  m_asyncStart(0),
  m_asyncEnd(0),
  m_blockingWrite(false),
  m_fifoReadName(pipeIn.empty() ? "" : s_fifoPath + s_fifoBaseName + pipeIn),
  m_fifoWriteName(pipeOut.empty() ? "" : s_fifoPath + s_fifoBaseName + pipeOut),
  m_inCreated(false),
  m_outCreated(false)
{
  // Make sure the fifo path exists
  if(mkdir(s_fifoPath.c_str(), 0777) != 0 && errno != EEXIST)
    throw std::bad_syscall("mkdir", lastError());

  try
  {
    if(!m_fifoReadName.empty())
    {
      if(createIn)
      {
        if(mkfifo(m_fifoReadName.c_str(), 0777) != 0) // TODO: permissions?
        {
          if(errno != EEXIST)
            throw std::bad_syscall("mkfifo", lastError());
        }
        else
          m_inCreated = true;
      }

      m_pipeRead = open(m_fifoReadName.c_str(), O_RDWR | O_NONBLOCK);
      if(m_pipeRead == INVALID_HANDLE_VALUE)
        throw std::bad_syscall("open", lastError() + ", " + m_fifoReadName);

      if(!setCloseOnExec(m_pipeRead))
        throw std::bad_syscall("fcntl", lastError());
    }

    if(!m_fifoWriteName.empty())
    {
      if(createOut)
      {
        if(mkfifo(m_fifoWriteName.c_str(), 0777) != 0)
        {
          if(errno != EEXIST)
            throw std::bad_syscall("mkfifo", lastError());
        }
        else
          m_outCreated = true;
      }

      m_pipeWrite = open(m_fifoWriteName.c_str(), O_RDWR | O_NONBLOCK);
      if(m_pipeWrite == INVALID_HANDLE_VALUE)
        throw std::bad_syscall("open", lastError() + ", " + m_fifoWriteName);

      if(!setCloseOnExec(m_pipeRead))
        throw std::bad_syscall("fcntl", lastError());
    }

    m_waitObject = new WaitObject(m_pipeRead);
  }
  catch(...)
  {
    cleanup();
    throw;
  }

  // Prepare async event structures
  memset(m_asyncArray, 0, sizeof(m_asyncArray));
  for(uint32_t i = 0; i < s_maxAsyncEvents; ++i)
  {
    m_asyncArray[i].aio_fildes = m_pipeWrite;
  }
}

LinuxPipe::LinuxPipe(Handle pipeRead, Handle pipeWrite) :
  m_waitObject(NULL),
  m_pipeRead(pipeRead),
  m_pipeWrite(pipeWrite),
  m_asyncStart(0),
  m_asyncEnd(0),
  m_blockingWrite(false),
  m_inCreated(false),
  m_outCreated(false)
{
  try
  {
    if(fcntl(m_pipeRead, F_SETFL, O_NONBLOCK) != 0 ||
       fcntl(m_pipeWrite, F_SETFL, O_NONBLOCK) != 0)
      throw std::bad_syscall("fcntl", lastError());

    m_waitObject = new WaitObject(m_pipeRead);
  }
  catch(...)
  {
    cleanup();
    throw;
  }

  // Prepare async event structures
  memset(m_asyncArray, 0, sizeof(m_asyncArray));
  for(uint32_t i = 0; i < s_maxAsyncEvents; ++i)
  {
    m_asyncArray[i].aio_fildes = m_pipeWrite;
  }
}

LinuxPipe::~LinuxPipe()
{
  // Destroy any asynchronous events and delete buffers
  aio_cancel(m_pipeWrite, NULL);

  for(uint32_t i = m_asyncStart; i != m_asyncEnd; i = (i + 1) % s_maxAsyncEvents)
  {
    delete [] reinterpret_cast<volatile uint8_t*>(m_asyncArray[i].aio_buf);
  }

  cleanup();
}

bool LinuxPipe::flush(uint32_t timeout)
{
  uint64_t endTime = getEndTime(timeout);
  getAsyncResults();

  // TODO: polling sucks, maybe rewrite to use kernel-based aio with eventfd rather than libc-based aio
  while(m_asyncStart != m_asyncEnd)
  {
    if(getTimeout(endTime) != 0)
      return false;

    sleep_ms(10);
    getAsyncResults();
  }

  return true;
}

void LinuxPipe::cleanup()
{
  if(m_pipeRead != INVALID_HANDLE_VALUE)
    close(m_pipeRead);

  if(m_pipeWrite != INVALID_HANDLE_VALUE)
    close(m_pipeWrite);

  if(m_inCreated)
    unlink(m_fifoReadName.c_str());

  if(m_outCreated && m_fifoReadName != m_fifoWriteName)
      unlink(m_fifoWriteName.c_str());

  delete m_waitObject;
  m_waitObject = NULL;
}

const std::string& LinuxPipe::getNameIn() const
{
  return m_fifoReadName;
}

const std::string& LinuxPipe::getNameOut() const
{
  return m_fifoWriteName;
}

void LinuxPipe::getAsyncResults()
{
  for(uint32_t i = m_asyncStart; i != m_asyncEnd; i = (i + 1) % s_maxAsyncEvents)
  {
    int asyncResult = aio_error(&m_asyncArray[i]);

    if(asyncResult == EINPROGRESS || asyncResult == EAGAIN)
      break;

    aio_return(&m_asyncArray[i]);
    delete [] reinterpret_cast<volatile uint8_t*>(m_asyncArray[i].aio_buf);
    m_asyncStart = (m_asyncStart + 1) % s_maxAsyncEvents;
  }
}

void LinuxPipe::asyncWrite(const void* buffer, uint32_t bufferSize)
{
  if(m_asyncStart == (m_asyncEnd + 1) % s_maxAsyncEvents)
    throw std::bad_alloc();

  struct aiocb* asyncEvent = &m_asyncArray[m_asyncEnd];

  asyncEvent->aio_buf = new uint8_t[bufferSize];
  asyncEvent->aio_nbytes = bufferSize;

  memcpy(const_cast<void*>(asyncEvent->aio_buf), buffer, bufferSize);

  if(aio_write(asyncEvent) != 0)
  {
    delete [] reinterpret_cast<volatile uint8_t*>(asyncEvent->aio_buf);
    throw std::bad_syscall("aio_write", lastError());
  }

  m_asyncEnd = (m_asyncEnd + 1) % s_maxAsyncEvents;
}

void LinuxPipe::send(const void* buffer, uint32_t bufferSize)
{
  getAsyncResults();

  // If we've still got asynchronous operations waiting, queue this up
  if(m_asyncStart != m_asyncEnd)
    return asyncWrite(buffer, bufferSize);

  if(m_blockingWrite)
  {
    fcntl(m_pipeWrite, F_SETFL, O_NONBLOCK);
    m_blockingWrite = false;
  }

  // Write as much as we can to the pipe, enqueue the rest asynchronously
  int bytesWritten = write(m_pipeWrite, buffer, bufferSize);

  if(bytesWritten < 0)
  {
    if(errno == EAGAIN)
      bytesWritten = 0;
    else
      throw std::bad_syscall("write to pipe", lastError());
  }

  if(static_cast<uint32_t>(bytesWritten) < bufferSize)
  {
    m_blockingWrite = true;
    fcntl(m_pipeWrite, F_SETFL, 0);
    asyncWrite(((uint8_t*)buffer) + bytesWritten, bufferSize - bytesWritten);
  }
}

uint32_t LinuxPipe::receive(void* buffer, uint32_t bufferSize)
{
  ssize_t bytesRead(read(m_pipeRead, buffer, bufferSize));

  if(bytesRead < 0)
  {
    if(errno != EAGAIN)
      throw std::bad_syscall("read from pipe", lastError());
    else
      bytesRead = 0;
  }

  return bytesRead;
}

LinuxPipe::operator WaitObject&()
{
  return *m_waitObject;
}

Handle LinuxPipe::getHandle() const
{
  return m_waitObject->getHandle();
}
