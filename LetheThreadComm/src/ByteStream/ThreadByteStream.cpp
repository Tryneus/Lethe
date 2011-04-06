#include "Lethe.h"
#include "ByteStream/ThreadByteStream.h"

using namespace lethe;

ThreadByteStream::ThreadByteStream(Pipe& pipeIn, Pipe& pipeOut) :
  m_pipeIn(pipeIn),
  m_pipeOut(pipeOut)
{
  // Do nothing
}

ThreadByteStream::~ThreadByteStream()
{
  // Do nothing
}

ThreadByteStream::operator WaitObject&()
{
  return m_pipeIn;
}

Handle ThreadByteStream::getHandle() const
{
  return m_pipeIn.getHandle();
}

void ThreadByteStream::send(const void* buffer, uint32_t size)
{
  m_pipeOut.send(buffer, size);
}

uint32_t ThreadByteStream::receive(void* buffer, uint32_t size)
{
  return m_pipeIn.receive(buffer, size);
}

