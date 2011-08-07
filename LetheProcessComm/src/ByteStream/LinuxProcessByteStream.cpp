#include "Lethe.h"
#include "LetheInternal.h"
#include "ByteStream/LinuxProcessByteStream.h"
#include "LinuxHandleTransfer.h"
#include "mct/hash-map.hpp"
#include <sstream>
#include "Log.h"

using namespace lethe;

Mutex LinuxProcessByteStream::s_mutex(false);
mct::closed_hash_map<uint32_t, Mutex*>* LinuxProcessByteStream::s_processInfo = NULL;

LinuxProcessByteStream::LinuxProcessByteStream(uint32_t remoteProcessId,
                                               uint32_t timeout) :
  m_pipeIn(NULL),
  m_pipeOut(NULL)
{
  // Don't allow a ProcessByteStream to be create towards own process
  // TODO: implement a workaround for this?
  if(remoteProcessId == getProcessId())
    throw std::logic_error("cannot create a ProcessByteStream within a single process");

  uint32_t endTime = getEndTime(timeout);
  std::stringstream fifoInName;
  std::stringstream fifoOutName;
  Mutex* processMutex = getProcessMutex(remoteProcessId);

  processMutex->lock(timeout);

  fifoInName << getProcessId() << "-to-" << remoteProcessId;
  fifoOutName << remoteProcessId << "-to-" << getProcessId();

  Pipe tempFifo(fifoInName.str(), true, fifoOutName.str(), true);

  doSetup(tempFifo, endTime);
}

LinuxProcessByteStream::LinuxProcessByteStream(ByteStream& stream,
                                               uint32_t timeout) :
  m_pipeIn(NULL),
  m_pipeOut(NULL)
{
  doSetup(stream, getEndTime(timeout));
}

void LinuxProcessByteStream::doSetup(ByteStream& stream,
                                     uint32_t endTime)
{
  LinuxHandleTransfer transfer(stream, getTimeout(endTime));

  m_pipeOut = new Pipe();
  transfer.sendPipe(*m_pipeOut);

  m_pipeIn = transfer.recvPipe(getTimeout(endTime));
}

LinuxProcessByteStream::~LinuxProcessByteStream()
{
  delete m_pipeOut;
  delete m_pipeIn;
}

LinuxProcessByteStream::operator WaitObject&()
{
  return *m_pipeIn;
}

Handle LinuxProcessByteStream::getHandle() const
{
  return m_pipeIn->getHandle();
}

void LinuxProcessByteStream::send(const void* buffer, uint32_t size)
{
  m_pipeOut->send(buffer, size);
}

uint32_t LinuxProcessByteStream::receive(void* buffer, uint32_t size)
{
  return m_pipeIn->receive(buffer, size);
}
