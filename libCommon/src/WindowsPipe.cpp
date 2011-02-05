#include "windows/WindowsPipe.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <Windows.h>

WindowsPipe::WindowsPipe() :
  WaitObject(INVALID_HANDLE_VALUE),
  m_pipeRead(INVALID_HANDLE_VALUE),
  m_pipeWrite(INVALID_HANDLE_VALUE),
  m_pendingData(NULL),
  m_pendingSend(NULL),
  m_pendingSize(0)
{
  if(!CreatePipe(&m_pipeRead, &m_pipeWrite, NULL, 16384))
    throw Exception("Failed to create pipe: " + lastError());

  DWORD nonBlocking(PIPE_NOWAIT);

  if(!SetNamedPipeHandleState(m_pipeRead, &nonBlocking, NULL, NULL) ||
     !SetNamedPipeHandleState(m_pipeWrite, &nonBlocking, NULL, NULL))
  {
    std::string errorString(lastError());
    CloseHandle(m_pipeRead);
    CloseHandle(m_pipeWrite);
    throw Exception("Failed to set pipe flags: " + errorString);
  }

  setWaitHandle(m_pipeRead);
}

WindowsPipe::~WindowsPipe()
{
  delete [] m_pendingData;

  if(!CloseHandle(m_pipeWrite))
  {
    CloseHandle(m_pipeRead);
    throw Exception("Failed to close pipe: " + lastError());
  }

  if(!CloseHandle(m_pipeRead))
    throw Exception("Failed to close pipe: " + lastError());

}

void WindowsPipe::send(uint8_t* buffer, uint32_t bufferSize)
{
  // Overlapped I/O would require a named pipe =(
  DWORD bytesWritten(0);

  // If we failed a send before, try to send the remainder now
  if(m_pendingData != NULL)
  {
    if(!WriteFile(m_pipeWrite, m_pendingSend, m_pendingSize, &bytesWritten, NULL))
      throw Exception("Failed to write to pipe: " + lastError());

    if(bytesWritten != m_pendingSize)
    {
      if(bytesWritten < 0)
      {
        m_pendingSend += bytesWritten;
        m_pendingSize -= bytesWritten;
      }

      throw OutOfMemoryException("WindowsPipe");
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
    throw Exception("Failed to write to pipe: " + lastError());

  // TODO: this leaves the possibility of an incomplete message if there is not active traffic
  // This should only happen on chunks of data larger than the pipe buffer, though
  if(bytesWritten < bufferSize)
  {
    // The write was not complete, store the remainder to send later
    m_pendingData = new uint8_t[bufferSize - bytesWritten];
    m_pendingSend = m_pendingData;
    m_pendingSize = bufferSize - bytesWritten;
    memcpy(m_pendingSend, buffer + bytesWritten, m_pendingSize);
  }
}

uint32_t WindowsPipe::receive(uint8_t* buffer, uint32_t bufferSize)
{
  DWORD bytesRead(0);

  if(!ReadFile(m_pipeRead, buffer, bufferSize, &bytesRead, NULL) &&
      GetLastError() != ERROR_MORE_DATA &&
      GetLastError() != ERROR_NO_DATA)
    throw Exception("Failed to read from pipe: " + lastError());

  return bytesRead;
}

