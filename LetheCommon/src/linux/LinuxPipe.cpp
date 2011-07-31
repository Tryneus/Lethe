#include "linux/LinuxPipe.h"
#include "LetheTypes.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <queue>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <aio.h>

using namespace lethe;

const std::string LinuxPipe::s_fifoPath("/tmp/lethe/");
const std::string LinuxPipe::s_fifoBaseName("lethe-fifo-");

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
  int pipes[2];

  try
  {
    if(pipe(pipes) != 0 ||
       pipes[0] == INVALID_HANDLE_VALUE ||
       pipes[1] == INVALID_HANDLE_VALUE)
      throw std::bad_syscall("pipe", lastError());

    m_pipeRead = pipes[0];
    m_pipeWrite = pipes[1];

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

LinuxPipe::LinuxPipe(const std::string& pipeIn, bool createIn, const std::string& pipeOut, bool createOut) :
  m_waitObject(NULL),
  m_pipeRead(INVALID_HANDLE_VALUE),
  m_pipeWrite(INVALID_HANDLE_VALUE),
  m_asyncStart(0),
  m_asyncEnd(0),
  m_blockingWrite(false),
  m_fifoReadName(pipeIn.empty() ? "" : s_fifoPath + s_fifoBaseName + pipeIn),
  m_fifoWriteName(pipeOut.empty() ? "" : s_fifoPath + s_fifoBaseName + pipeOut),
  m_inCreated(createIn),
  m_outCreated(createOut)
{
  // Make sure the fifo path exists
  if(mkdir(s_fifoPath.c_str(), 0777) != 0 && errno != EEXIST)
    throw std::bad_syscall("mkdir", lastError());

  try
  {
    if(!m_fifoReadName.empty())
    {
      if(createIn && mkfifo(m_fifoReadName.c_str(), 0777) != 0 && errno != EEXIST) // TODO: permissions?
        throw std::bad_syscall("mkfifo", lastError());

      m_pipeRead = open(m_fifoReadName.c_str(), O_RDWR | O_NONBLOCK);
      if(m_pipeRead == INVALID_HANDLE_VALUE)
        throw std::bad_syscall("open", lastError() + ", " + m_fifoReadName);
    }

    if(!m_fifoWriteName.empty())
    {
      if(createOut && mkfifo(m_fifoWriteName.c_str(), 0777) != 0 && errno != EEXIST)
        throw std::bad_syscall("mkfifo", lastError());

      m_pipeWrite = open(m_fifoWriteName.c_str(), O_RDWR | O_NONBLOCK);
      if(m_pipeWrite == INVALID_HANDLE_VALUE)
        throw std::bad_syscall("open", lastError() + ", " + m_fifoWriteName);
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
  std::queue<aiocb*> unfinishedEvents;

  // Destroy any asynchronous events and delete buffers
  for(uint32_t i = m_asyncStart; i != m_asyncEnd; i = (i + 1) % s_maxAsyncEvents)
  {
    if(aio_cancel(m_pipeWrite, &m_asyncArray[i]) != AIO_CANCELED)
      unfinishedEvents.push(&m_asyncArray[i]);
    else
      delete [] reinterpret_cast<volatile uint8_t*>(m_asyncArray[i].aio_buf);
  }

  cleanup();

  while(!unfinishedEvents.empty())
  {
    int status = EINPROGRESS;

    // TODO: polling sucks, maybe rewrite to use kernel-based aio with eventfd rather than libc-based aio
    while(status == EINPROGRESS || status == EAGAIN)
    {
      sleep_ms(5);
      status = aio_error(unfinishedEvents.front());
    }

    delete [] reinterpret_cast<volatile uint8_t*>(unfinishedEvents.front()->aio_buf);
    aio_return(unfinishedEvents.front());
    unfinishedEvents.pop();
  }
}

void LinuxPipe::cleanup()
{
  if(m_pipeRead != INVALID_HANDLE_VALUE)
    close(m_pipeRead);

  if(m_pipeWrite != INVALID_HANDLE_VALUE)
    close(m_pipeWrite);

  if(!m_fifoReadName.empty() && m_inCreated)
    unlink(m_fifoReadName.c_str());

  if(!m_fifoWriteName.empty() && m_outCreated)
    unlink(m_fifoWriteName.c_str());

  delete m_waitObject;
  m_waitObject = NULL;
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
    throw std::bad_syscall("read from pipe", lastError());

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
