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
  ByteStream(INVALID_HANDLE_VALUE),
  m_pipeRead(INVALID_HANDLE_VALUE),
  m_pipeWrite(INVALID_HANDLE_VALUE),
  m_inCreated(false),
  m_outCreated(false)
{
  setupAsync();

  // No name provided, auto-generate one
  std::stringstream str;
  str << s_fifoPath << s_fifoBaseName << getProcessId() << "-" << s_uniqueId.increment();
  m_fifoReadName.assign(str.str());
  m_fifoWriteName.assign(str.str());

  try
  {
    // Make sure the fifo path exists
    if(mkdir(s_fifoPath.c_str(), 0777) != 0 && errno != EEXIST)
      throw std::bad_syscall("mkdir", lastError());

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
  }
  catch(...)
  {
    cleanup();
    throw;
  }

  setHandle(m_pipeRead);
}

LinuxPipe::LinuxPipe(const std::string& pipeIn, bool createIn, const std::string& pipeOut, bool createOut) :
  ByteStream(INVALID_HANDLE_VALUE),
  m_pipeRead(INVALID_HANDLE_VALUE),
  m_pipeWrite(INVALID_HANDLE_VALUE),
  m_fifoReadName(pipeIn.empty() ? "" : s_fifoPath + s_fifoBaseName + pipeIn),
  m_fifoWriteName(pipeOut.empty() ? "" : s_fifoPath + s_fifoBaseName + pipeOut),
  m_inCreated(false),
  m_outCreated(false)
{
  setupAsync();

  try
  {
    // Make sure the fifo path exists
    if(mkdir(s_fifoPath.c_str(), 0777) != 0 && errno != EEXIST)
      throw std::bad_syscall("mkdir", lastError());

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
  }
  catch(...)
  {
    cleanup();
    throw;
  }

  setHandle(m_pipeRead);
}

LinuxPipe::LinuxPipe(Handle pipeRead, Handle pipeWrite) :
  ByteStream(INVALID_HANDLE_VALUE),
  m_pipeRead(pipeRead),
  m_pipeWrite(pipeWrite),
  m_inCreated(false),
  m_outCreated(false)
{
  setupAsync();

  try
  {
    if(fcntl(m_pipeRead, F_SETFL, O_NONBLOCK) != 0 ||
       fcntl(m_pipeWrite, F_SETFL, O_NONBLOCK) != 0)
      throw std::bad_syscall("fcntl", lastError());

    if(!setCloseOnExec(m_pipeRead))
      throw std::bad_syscall("fcntl", lastError());

    if(!setCloseOnExec(m_pipeWrite))
      throw std::bad_syscall("fcntl", lastError());
  }
  catch(...)
  {
    cleanup();
    throw;
  }

  setHandle(m_pipeRead);
}

void LinuxPipe::setupAsync()
{
  if(pthread_attr_init(&m_async.attr) != 0)
    throw std::bad_syscall("pthread_attr_init", lastError());

  if(pthread_attr_setdetachstate(&m_async.attr, PTHREAD_CREATE_DETACHED) != 0)
  {
    pthread_attr_destroy(&m_async.attr);
    throw std::bad_syscall("pthread_attr_setdetachstate", lastError());
  }

  m_async.buffer = NULL;
  m_async.result = 0;
}

LinuxPipe::~LinuxPipe()
{
  // Destroy any asynchronous events and delete buffers
  if(m_async.buffer != NULL)
  {
    m_async.result = EINTR;

    try
    {
      flush(30);
    }
    catch(std::bad_syscall&)
    {
      // Do nothing
    }

    delete [] reinterpret_cast<volatile uint8_t*>(m_async.buffer);
  }

  cleanup();
}

bool LinuxPipe::flush(uint32_t timeout)
{
  if(m_async.buffer != NULL)
  {
    uint64_t endTime = getEndTime(timeout);

    // Polling sucks, but can't get kernel eventfd-aio or libc aio to work with a full pipe
    do
    {
      timeout = getTimeout(endTime);

      if(timeout == 0)
        return false;

      sleep_ms((20 < timeout) ? 20 : timeout);
    } while(m_async.buffer != NULL);
  }

  if(m_async.result != 0)
    throw std::bad_syscall("write", getErrorString(m_async.result));

  return true;
}

void LinuxPipe::cleanup()
{
  pthread_attr_destroy(&m_async.attr);

  if(m_pipeRead != INVALID_HANDLE_VALUE)
    close(m_pipeRead);

  if(m_pipeWrite != INVALID_HANDLE_VALUE)
    close(m_pipeWrite);

  if(m_inCreated)
    unlink(m_fifoReadName.c_str());

  if(m_outCreated && m_fifoReadName != m_fifoWriteName)
    unlink(m_fifoWriteName.c_str());
}

const std::string& LinuxPipe::getNameIn() const
{
  return m_fifoReadName;
}

const std::string& LinuxPipe::getNameOut() const
{
  return m_fifoWriteName;
}

void LinuxPipe::send(const void* buffer, uint32_t bufferSize)
{
  // Check if a previous async write has failed
  if(m_async.result != 0)
    throw std::bad_syscall("write", getErrorString(m_async.result));

  // If there is currently a running async write, don't allow a new write
  if(m_async.buffer != NULL)
    throw std::bad_alloc();

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
    startAsync(((uint8_t*)buffer) + bytesWritten, bufferSize - bytesWritten);
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

void LinuxPipe::startAsync(uint8_t* buffer, uint32_t size)
{
  m_async.buffer = new uint8_t[size];
  m_async.size = size;
  m_async.offset = 0;

  memcpy(m_async.buffer, buffer, size);

  if(pthread_create(&m_async.thread, &m_async.attr, &asyncThreadHook, this) != 0)
    throw std::bad_syscall("pthread_create", lastError());
}

void* LinuxPipe::asyncThreadHook(void* param)
{
  LinuxPipe* pipe = reinterpret_cast<LinuxPipe*>(param);
  pipe->asyncThreadInternal();
  return NULL;
}

void LinuxPipe::asyncThreadInternal()
{
  while(m_async.result == 0)
  {
    int bytesWritten = write(m_pipeWrite, &m_async.buffer[m_async.offset], m_async.size);

    if(bytesWritten > 0)
    {
      m_async.offset += bytesWritten;
      m_async.size -= bytesWritten;
    }
    else if(bytesWritten == 0 || errno == EAGAIN)
      sleep_ms(20);
    else
      m_async.result = errno;
  }

  delete [] m_async.buffer;
  m_async.buffer = NULL;
}
