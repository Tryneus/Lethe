#include "linux/LinuxPipe.h"
#include "AbstractionTypes.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include <queue>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <aio.h>

LinuxPipe::LinuxPipe() :
  WaitObject(INVALID_HANDLE_VALUE),
  m_pipeRead(INVALID_HANDLE_VALUE),
  m_pipeWrite(INVALID_HANDLE_VALUE),
  m_asyncStart(0),
  m_asyncEnd(0),
  m_blockingWrite(false)
{
  int pipes[2];

  if(pipe(pipes) != 0 ||
     pipes[0] == INVALID_HANDLE_VALUE ||
     pipes[1] == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("pipe", lastError());

  m_pipeRead = pipes[0];
  m_pipeWrite = pipes[1];

  if(fcntl(m_pipeRead, F_SETFL, O_NONBLOCK) != 0 ||
     fcntl(m_pipeWrite, F_SETFL, O_NONBLOCK) != 0)
  {
    close(m_pipeRead);
    close(m_pipeWrite);
    throw std::bad_syscall("fcntl", lastError()); // TODO: this probably won't have the right errno
  }

  setWaitHandle(m_pipeRead);

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

  close(m_pipeWrite);
  close(m_pipeRead);

  while(!unfinishedEvents.empty())
  {
    int status = EINPROGRESS;

    // TODO: polling sucks, maybe rewrite to use kernel-based aio with eventfd rather than libc-based aio
    while(status == EINPROGRESS || status == EAGAIN)
    {
      Sleep(5);
      status = aio_error(unfinishedEvents.front());
    }

    delete [] reinterpret_cast<volatile uint8_t*>(unfinishedEvents.front()->aio_buf);
    aio_return(unfinishedEvents.front());
    unfinishedEvents.pop();
  }
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
