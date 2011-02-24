#include "MessageStream/ThreadMessageConnection.h"
#include "MessageStream/ThreadMessage.h"
#include "AbstractionException.h"

using namespace comm;

ThreadMessageConnection::ThreadMessageConnection(uint32_t sizeAtoB, uint32_t sizeBtoA) :
  m_semaphoreAtoB(sizeAtoB / sizeof(ThreadMessage), 0),
  m_semaphoreBtoA(sizeBtoA / sizeof(ThreadMessage), 0),
  m_headerAtoB(checkSize(sizeAtoB), m_semaphoreAtoB),
  m_headerBtoA(checkSize(sizeBtoA), m_semaphoreBtoA),
  m_streamA(m_headerBtoA, m_headerAtoB, m_semaphoreBtoA),
  m_streamB(m_headerAtoB, m_headerBtoA, m_semaphoreAtoB)
{
  // Do nothing
}

ThreadMessageConnection::~ThreadMessageConnection()
{
  // Do nothing
}

uint32_t ThreadMessageConnection::checkSize(uint32_t size)
{
  if(size < s_minSize)
    throw std::invalid_argument("size too small for communication");

  if(size > s_maxSize)
    size = s_maxSize;

  return size;
}

MessageStream& ThreadMessageConnection::getStreamA()
{
  return m_streamA;
}

MessageStream& ThreadMessageConnection::getStreamB()
{
  return m_streamB;
}
