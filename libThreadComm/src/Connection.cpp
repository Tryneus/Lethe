#include "Connection.h"
#include "AbstractionException.h"

using namespace ThreadComm;

Connection::Connection(uint32_t sizeAtoB, uint32_t sizeBtoA) :
  m_headerAtoB(checkSize(sizeAtoB)),
  m_headerBtoA(checkSize(sizeBtoA)),
  m_channelA(m_headerBtoA, m_headerAtoB),
  m_channelB(m_headerAtoB, m_headerBtoA)
{
  // Do nothing
}

Connection::~Connection()
{
  // Do nothing
}

uint32_t Connection::checkSize(uint32_t size)
{
  if(size < s_minSize)
    throw std::invalid_argument("size too small for communication");

  if(size > s_maxSize)
    size = s_maxSize;

  return size;
}

Channel& Connection::getChannelA()
{
  return m_channelA;
}

Channel& Connection::getChannelB()
{
  return m_channelB;
}
