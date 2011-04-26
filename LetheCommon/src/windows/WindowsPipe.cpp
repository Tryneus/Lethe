#include "windows/WindowsPipe.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <Windows.h>
#include <sstream>

using namespace lethe;

//const uint32_t WindowsPipe::s_maxAsyncEvents = 10;
DWORD WindowsPipe::s_procId = 0;
uint32_t WindowsPipe::s_uniqueId = 0;

const std::string WindowsPipe::s_pipeBaseName("\\\\.\\pipe\\lethe-pipe-");
const std::string WindowsPipe::s_eventBaseName("\\Global\\lethe-pipe-event-");
const std::string WindowsPipe::s_mutexBaseName("\\Global\\lethe-pipe-mutex-");

WindowsPipe::WindowsPipe() :
  m_waitObject(INVALID_HANDLE_VALUE),
  m_pipeInName(generatePipeName()),
  m_pipeOutName(m_pipeInName),
  m_mutex(false),
  m_dataInEvent(false, false),
  m_dataOutEvent(false, false),
  m_dataCount(0),
  m_pipeRead(INVALID_HANDLE_VALUE),
  m_pipeWrite(INVALID_HANDLE_VALUE),
  m_asyncStart(0),
  m_asyncEnd(0),
  m_isBlocking(true)
{
  try
  {
    m_pipeWrite = createPipe(m_pipeNameOut);
    m_pipeRead = openPipe(m_pipeNameOut);

    DWORD state(PIPE_NOWAIT);
    if(!SetNamedPipeHandleState(m_pipeRead, &state, NULL, NULL))
      throw std::bad_syscall("SetNamedPipeHandleState", lastError());
  }
  catch(...)
  {
    cleanup();
    throw;
  }

  setWaitHandle(m_dataEvent.getHandle());
}

WindowsPipe::WindowsPipe(const std::string& pipeIn, bool createIn, const std::string& pipeOut, bool createOut) :
  m_waitObject(INVALID_HANDLE_VALUE),
  m_pipeNameIn(s_pipeBaseName + pipeIn),
  m_pipeNameOut(s_pipeBaseName + pipeOut),
  m_mutex(false),
  m_dataInEvent(false, false, s_eventBaseName + pipeIn),
  m_dataOutEvent(false, false, s_eventBaseName + pipeOut),
  m_dataCount(0),
  m_pipeRead(INVALID_HANDLE_VALUE),
  m_pipeWrite(INVALID_HANDLE_VALUE),
  m_asyncStart(0),
  m_asyncEnd(0),
  m_isBlocking(true)
{
  try
  {
    if(createIn)
      m_pipeRead = createPipe(m_pipeNameIn);
    else
      m_pipeRead = openPipe(m_pipeNameIn);

    if(createOut)
      m_pipeWrite = createPipe(m_pipeNameOut);
    else
      m_pipeWrite = openPipe(m_pipeNameOut);

    DWORD state(PIPE_NOWAIT);
    if(!SetNamedPipeHandleState(m_pipeIn, &state, NULL, NULL))
      throw std::bad_syscall("SetNamedPipeHandleState", errorString);
  }
  catch(...)
  {
    cleanup();
    throw;
  }

  setWaitHandle(m_dataEvent.getHandle());
}

Handle WindowsPipe::createPipe(const std::string& name)
{
  Handle handle = CreateNamedPipe(name.c_str(),
                                  PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                  PIPE_TYPE_BYTE | PIPE_NOWAIT | PIPE_REJECT_REMOTE_CLIENTS,
                                  1,
                                  65536, // Outgoing buffer size
                                  0,     // Incoming buffer size
                                  0,
                                  NULL);

  if(handle == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("CreateNamedPipe", lastError());

  return handle;
}

Handle WindowsPipe::openPipe(const std::string& name)
{
  Handle handle = CreateFile(name.c_str(),
                             GENERIC_READ | GENERIC_WRITE,
                             0, // Will this conflict with the created pipe?
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL);

  if(handle == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("CreateFile", lastError());

  return handle;
}

void WindowsPipe::cleanup()
{
  if(m_pipeWrite != INVALID_HANDLE_VALUE)
    CloseHandle(m_pipeWrite);

  if(m_pipeRead != INVALID_HANDLE_VALUE)
    CloseHandle(m_pipeRead);
}

WindowsPipe::~WindowsPipe()
{
  getAsyncResults();

  for(uint32_t i = m_asyncStart; i != m_asyncEnd; i = (i + 1) % s_maxAsyncEvents)
  {
    CancelIoEx(m_pipeWrite, &m_asyncArray[i].overlapped);
    delete [] m_asyncArray[i].buffer;
  }

  cleanup();
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

    m_isBlocking = false;
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

    m_isBlocking = true;

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

std::string WindowsPipe::generatePipeName()
{
  std::stringstream pipeName;
  pipeName << s_pipeBaseName << getProcessId() << "-" << s_uniqueId++;
  return pipeName.str().c_str();
}
