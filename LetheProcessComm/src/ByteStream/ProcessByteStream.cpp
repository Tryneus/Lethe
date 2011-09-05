#include "Lethe.h"
#include "LetheInternal.h"
#include "ProcessComm.h"
#include <sstream>

using namespace lethe;

ProcessByteStream::ProcessByteStream(uint32_t remoteProcessId,
                                     uint32_t timeout) :
  ByteStream(INVALID_HANDLE_VALUE),
  m_pipeIn(NULL),
  m_pipeOut(new Pipe())
{
  uint64_t endTime = getEndTime(timeout);
  HandleTransfer transfer(remoteProcessId, getTimeout(endTime));

  transfer.sendPipe(*m_pipeOut);
  m_pipeIn = transfer.recvPipe(getTimeout(endTime));

  setHandle(m_pipeIn->getHandle());
}

ProcessByteStream::ProcessByteStream(ByteStream& stream,
                                     uint32_t timeout) :
  ByteStream(INVALID_HANDLE_VALUE),
  m_pipeIn(NULL),
  m_pipeOut(new Pipe())
{
  uint64_t endTime = getEndTime(timeout);
  HandleTransfer transfer(stream, getTimeout(endTime));

  transfer.sendPipe(*m_pipeOut);
  m_pipeIn = transfer.recvPipe(getTimeout(endTime));

  setHandle(m_pipeIn->getHandle());
}

ProcessByteStream::~ProcessByteStream()
{
  delete m_pipeOut;
  delete m_pipeIn;
}

bool ProcessByteStream::flush(uint32_t timeout)
{
  return m_pipeOut->flush(timeout);
}

void ProcessByteStream::send(const void* buffer, uint32_t size)
{
  m_pipeOut->send(buffer, size);
}

uint32_t ProcessByteStream::receive(void* buffer, uint32_t size)
{
  return m_pipeIn->receive(buffer, size);
}
