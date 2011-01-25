#include "WindowsPipe.h"
#include "Exception.h"

WindowsPipe::WindowsPipe() :
  m_pipe(INVALID_HANDLE_VALUE)
{
  if(m_pipe == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create pipe: " + lastError());
}

WindowsPipe::~WindowsPipe()
{
  if(!CloseHandle(m_pipe))
    throw Exception("Failed to close pipe: " + lastError());
}

uint32_t WindowsPipe::send()
{
  WriteFile(m_pipe, buffer, countIn, countOut, NULL);
}

uint32_t WindowsPipe::receive()
{
  ReadFile(m_pipe, buffer, countIn, countOut, NULL);
}

HANDLE WindowsPipe::getHandle()
{
  return m_pipe;
}
