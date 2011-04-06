#include "Lethe.h"
#include "ByteStream/ThreadByteConnection.h"

using namespace lethe;

ThreadByteConnection::ThreadByteConnection() :
  m_streamA(m_pipeBtoA, m_pipeAtoB),
  m_streamB(m_pipeAtoB, m_pipeBtoA)
{
  // Do nothing
}

ThreadByteConnection::~ThreadByteConnection()
{
  // Do nothing
}

ByteStream& ThreadByteConnection::getStreamA()
{
  return m_streamA;
}

ByteStream& ThreadByteConnection::getStreamB()
{
  return m_streamB;
}

