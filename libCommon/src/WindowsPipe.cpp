#include "windows/WindowsPipe.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include <Windows.h>

WindowsPipe::WindowsPipe() :
  WaitObject(INVALID_HANDLE_VALUE),
  m_mutex(false),
  m_dataEvent(false, false),
  m_dataCount(0),
  m_pipeRead(INVALID_HANDLE_VALUE),
  m_pipeWrite(INVALID_HANDLE_VALUE),
  m_pendingData(NULL),
  m_pendingSend(NULL),
  m_pendingSize(0)
{
  if(!CreatePipe(&m_pipeRead, &m_pipeWrite, NULL, 16384))
    throw std::bad_syscall("CreatePipe", lastError());

  DWORD nonBlocking(PIPE_NOWAIT);

  if(!SetNamedPipeHandleState(m_pipeRead, &nonBlocking, NULL, NULL) ||
     !SetNamedPipeHandleState(m_pipeWrite, &nonBlocking, NULL, NULL))
  {
    std::string errorString(lastError());
    CloseHandle(m_pipeRead);
    CloseHandle(m_pipeWrite);
    throw std::bad_syscall("SetNamedPipeHandleState", errorString);
  }

  setWaitHandle(m_dataEvent.getHandle());
}

WindowsPipe::~WindowsPipe()
{
  delete [] m_pendingData;

  CloseHandle(m_pipeWrite);
  CloseHandle(m_pipeRead);
  CloseHandle(m_mutex.getHandle());
  CloseHandle(m_dataEvent.getHandle());
}

void WindowsPipe::send(const void* buffer, uint32_t bufferSize)
{
  // Overlapped I/O would require a named pipe =(
  DWORD bytesWritten(0);

  // If we failed a send before, try to send the remainder now
  if(m_pendingData != NULL)
  {
    if(!WriteFile(m_pipeWrite, m_pendingSend, m_pendingSize, &bytesWritten, NULL))
      throw std::bad_syscall("WriteFile", lastError());

    if(bytesWritten != 0)
      updateDataEvent(bytesWritten);

    if(bytesWritten != m_pendingSize)
    {
      if(bytesWritten > 0)
      {
        m_pendingSend += bytesWritten;
        m_pendingSize -= bytesWritten;
      }

      throw std::bad_alloc();
    }
    else
    {
      delete [] m_pendingData;
      m_pendingData = NULL;
      m_pendingSend = NULL;
      m_pendingSize = 0;
    }
  }

  if(!WriteFile(m_pipeWrite, buffer, bufferSize, &bytesWritten, NULL))
    throw std::bad_syscall("WriteFile", lastError());

  if(bytesWritten != 0)
    updateDataEvent(bytesWritten);

  // TODO: this leaves the possibility of an incomplete message if there is not active traffic
  // This should only happen on chunks of data larger than the pipe buffer, though
  if(bytesWritten < bufferSize)
  {
    // The write was not complete, store the remainder to send later
    m_pendingData = new uint8_t[bufferSize - bytesWritten];
    m_pendingSend = m_pendingData;
    m_pendingSize = bufferSize - bytesWritten;
    memcpy(m_pendingSend, reinterpret_cast<const uint8_t*>(buffer) + bytesWritten, m_pendingSize);
  }
}

void WindowsPipe::updateDataEvent(uint32_t bytesWritten)
{
  m_mutex.lock();
  m_dataCount += bytesWritten;
  m_dataEvent.set();
  m_mutex.unlock();
}

uint32_t WindowsPipe::receive(void* buffer, uint32_t bufferSize)
{
  DWORD bytesRead(0);

  if(!ReadFile(m_pipeRead, buffer, bufferSize, &bytesRead, NULL) &&
      GetLastError() != ERROR_MORE_DATA &&
      GetLastError() != ERROR_NO_DATA)
    throw std::bad_syscall("ReadFile", lastError());

  // Update the data event
  if(bytesRead > 0)
  {
    m_mutex.lock();
    m_dataCount -= bytesRead;
    if(m_dataCount == 0)
      m_dataEvent.reset();
    m_mutex.unlock();
  }

  return bytesRead;
}
