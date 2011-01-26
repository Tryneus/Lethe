#include "LinuxPipe.h"
#include "Exception.h"
#include "Abstraction.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

LinuxPipe::LinuxPipe() :
  m_pipeRead(-1),
  m_pipeWrite(-1),
  m_pendingData(NULL),
  m_pendingSend(NULL),
  m_pendingSize(0)
{
  int pipes[2];

  if(pipe(pipes) != 0)
    throw Exception("Failed to create pipe: " + lastError());

  m_pipeRead = pipes[0];
  m_pipeWrite = pipes[1];

  fcntl(m_pipeRead, F_SETFL, O_NONBLOCK);
  fcntl(m_pipeWrite, F_SETFL, O_NONBLOCK);
}

LinuxPipe::~LinuxPipe()
{
  delete [] m_pendingData;

  if(close(m_pipeWrite) != 0)
  {
    close(m_pipeRead);
    throw Exception("Failed to close pipe: " + lastError());
  }

  if(close(m_pipeRead) != 0)
    throw Exception("Failed to close pipe: " + lastError());

}

void LinuxPipe::send(uint8_t* buffer, uint32_t bufferSize)
{
  ssize_t bytesWritten;

  // If we failed a send before, try to send the remainder now
  if(m_pendingData != NULL)
  {
    bytesWritten = write(m_pipeWrite, m_pendingSend, m_pendingSize);

    if(bytesWritten != m_pendingSize)
    {
      if(bytesWritten != -1)
      {
        m_pendingSend += bytesWritten;
        m_pendingSize -= bytesWritten;
      }

      throw OutOfMemoryException("LinuxPipe");
    }
    else
    {
      delete [] m_pendingData;
      m_pendingData = NULL;
      m_pendingSend = NULL;
      m_pendingSize = 0;
    }
  }

  bytesWritten = write(m_pipeWrite, buffer, bufferSize);

  if(bytesWritten == -1)
    throw Exception("Failed to write to pipe: " + lastError());

  // TODO: this leaves the possibility of an incomplete message if there is not active traffic
  // This should only happen on chunks of data larger than the pipe buffer, though
  if(bytesWritten < static_cast<ssize_t>(bufferSize))
  {
    // The write was not complete, store the remainder to send later
    m_pendingData = new uint8_t[bufferSize - bytesWritten];
    m_pendingSend = m_pendingData;
    m_pendingSize = bufferSize - bytesWritten;
    memcpy(m_pendingSend, buffer + bytesWritten, m_pendingSize);
  }
}

uint32_t LinuxPipe::receive(uint8_t* buffer, uint32_t bufferSize)
{
  ssize_t bytesRead = read(m_pipeRead, buffer, bufferSize);

  if(bytesRead == -1)
    throw Exception("Failed to read from pipe: " + lastError());

  return bytesRead;
}

int LinuxPipe::getHandle()
{
  return m_pipeRead;
}
