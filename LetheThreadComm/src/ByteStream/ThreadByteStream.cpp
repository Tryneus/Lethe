#include "Lethe.h"
#include "ByteStream/ThreadByteStream.h"

using namespace lethe;

ThreadByteStream::ThreadByteStream(Pipe& pipeIn, Pipe& pipeOut) :
  ByteStream(INVALID_HANDLE_VALUE),
  m_pipeIn(pipeIn),
  m_pipeOut(pipeOut)
{
  setHandle(m_pipeIn.getHandle());
}

ThreadByteStream::~ThreadByteStream()
{
  // Do nothing
}

bool ThreadByteStream::flush(uint32_t timeout)
{
  return m_pipeOut.flush(timeout);
}

void ThreadByteStream::send(const void* buffer, uint32_t size)
{
  m_pipeOut.send(buffer, size);
}

uint32_t ThreadByteStream::receive(void* buffer, uint32_t size)
{
  return m_pipeIn.receive(buffer, size);
}

