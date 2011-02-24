#include "windows/WindowsPipe.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include <Windows.h>
#include <sstream>

//const uint32_t WindowsPipe::s_maxAsyncEvents = 10;
DWORD WindowsPipe::s_procId = 0;
uint32_t WindowsPipe::s_uniqueId = 0;

WindowsPipe::WindowsPipe() :
  WaitObject(INVALID_HANDLE_VALUE),
  m_pipeName(getPipeName()),
  m_mutex(false),
  m_dataEvent(false, false),
  m_dataCount(0),
  m_pipeRead(INVALID_HANDLE_VALUE),
  m_pipeWrite(INVALID_HANDLE_VALUE),
  m_asyncStart(0),
  m_asyncEnd(0),
  m_isBlocking(true)
{
  m_pipeWrite = CreateNamedPipe(m_pipeName.c_str(),
                  PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                  PIPE_TYPE_BYTE | PIPE_NOWAIT | PIPE_REJECT_REMOTE_CLIENTS,
                  1,
                  65536, // Outgoing buffer size
                  0,     // Incoming buffer size
                  0,
                  NULL);

  if(m_pipeWrite == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("CreateNamedPipe", lastError());

  m_pipeRead = CreateFile(m_pipeName.c_str(),
                  GENERIC_READ | GENERIC_WRITE,
                  0, // Will this conflict with the created pipe?
                  NULL,
                  OPEN_EXISTING,
                  0,
                  NULL);

  if(m_pipeRead == INVALID_HANDLE_VALUE)
  {
    std::string errorString(lastError());
    CloseHandle(m_pipeWrite);
    throw std::bad_syscall("CreateFile", errorString);
  }

  DWORD nonBlocking(PIPE_NOWAIT);

  if(!SetNamedPipeHandleState(m_pipeRead, &nonBlocking, NULL, NULL))
  {
    std::string errorString(lastError());
    CloseHandle(m_pipeWrite);
    CloseHandle(m_pipeRead);
    throw std::bad_syscall("SetNamedPipeHandleState", errorString);
  }

  setWaitHandle(m_dataEvent.getHandle());
}

WindowsPipe::~WindowsPipe()
{
  getAsyncResults();

  for(uint32_t i = m_asyncStart; i != m_asyncEnd; i = (i + 1) % s_maxAsyncEvents)
  {
    CancelIoEx(m_pipeWrite, &m_asyncArray[i].overlapped);
    delete [] m_asyncArray[i].buffer;
  }

  CloseHandle(m_pipeWrite);
  CloseHandle(m_pipeRead);
}

void WindowsPipe::getAsyncResults()
{
  for(uint32_t i = m_asyncStart; i != m_asyncEnd; i = (i + 1) % s_maxAsyncEvents)
  {
    if(!HasOverlappedIoCompleted(&m_asyncArray[i].overlapped))
      break;

    delete [] m_asyncArray[i].buffer;
    m_asyncStart = (m_asyncStart + 1) % s_maxAsyncEvents;
  }
}

void WindowsPipe::asyncWrite(const void* buffer, uint32_t bufferSize)
{
  if(m_asyncStart == (m_asyncEnd + 1) % s_maxAsyncEvents)
    throw std::bad_alloc();

  DWORD bytesWritten;
  OVERLAPPED* asyncEvent = &m_asyncArray[m_asyncEnd].overlapped;

  m_asyncArray[m_asyncEnd].buffer = new uint8_t[bufferSize];

  memset(asyncEvent, 0, sizeof(*asyncEvent));
  memcpy(m_asyncArray[m_asyncEnd].buffer, buffer, bufferSize);

  if(WriteFile(m_pipeWrite, buffer, bufferSize, &bytesWritten, asyncEvent))
  {
    if(bytesWritten != bufferSize)
      m_asyncEnd = (m_asyncEnd + 1) % s_maxAsyncEvents;
    else
      delete [] m_asyncArray[m_asyncEnd].buffer;
  }
  else if(GetLastError() == ERROR_IO_PENDING)
  {
    m_asyncEnd = (m_asyncEnd + 1) % s_maxAsyncEvents;
  }
  else
  {
    delete [] m_asyncArray[m_asyncEnd].buffer;
    throw std::bad_syscall("WriteFile", lastError());
  }

  updateDataEvent(bufferSize);
}

void WindowsPipe::send(const void* buffer, uint32_t bufferSize)
{
  getAsyncResults();

  // If we've still got asynchronous operations waiting, queue this up
  if(m_asyncStart != m_asyncEnd)
    return asyncWrite(buffer, bufferSize);

  if(m_isBlocking)
  {
    DWORD nonBlocking = PIPE_NOWAIT;
    if(!SetNamedPipeHandleState(m_pipeWrite, &nonBlocking, NULL, NULL))
      throw std::bad_syscall("SetNamedPipeHandleState", lastError());
  }

  DWORD bytesWritten;

  if(!WriteFile(m_pipeWrite, buffer, bufferSize, &bytesWritten, NULL))
    throw std::bad_syscall("WriteFile", lastError());

  if(bytesWritten != 0)
    updateDataEvent(bytesWritten);

  if(bytesWritten < bufferSize)
  {
    DWORD blocking = PIPE_WAIT;
    if(!SetNamedPipeHandleState(m_pipeWrite, &blocking, NULL, NULL))
      throw std::bad_syscall("SetNamedPipeHandleState", lastError());

    asyncWrite(((uint8_t*)buffer) + bytesWritten, bufferSize - bytesWritten);
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
  else
    throw std::bad_syscall("ReadFile", "no data to receive");

  return bytesRead;
}

std::string WindowsPipe::getPipeName()
{
  if(s_procId == 0)
    s_procId = GetCurrentProcessId();

  std::stringstream pipeName;
  pipeName << "\\\\.\\pipe\\" << s_procId << "-" << s_uniqueId++;
  return pipeName.str().c_str();
}
