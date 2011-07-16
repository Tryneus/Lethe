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

  char done = '\0';
  uint32_t endTime = getEndTime(timeout);
  std::stringstream fifoInName;
  std::stringstream fifoOutName;
  Mutex* processMutex = getProcessMutex(remoteProcessId);

  processMutex->lock(timeout);

  fifoInName << getProcessId() << "-to-" << remoteProcessId;
  fifoOutName << remoteProcessId << "-to-" << getProcessId();

  Pipe tempFifo(fifoInName.str(), true, fifoOutName.str(), true);

  doSetup(tempFifo, endTime);

  // Synchronize with other side to make sure everything is fine
  tempFifo.send(&done, 1);

  if(WaitForObject(tempFifo, getTimeout(endTime)) != WaitSuccess)
    throw std::runtime_error("ProcessByteStream timed out waiting for done indication");

  if(tempFifo.receive(&done, 1) != 1 || done != '\0')
    throw std::logic_error("ProcessByteStream received unexpected data");

  processMutex->unlock();

  // Try to remove the process info structure if it isn't being used by another thread
  removeProcessMutex(remoteProcessId);
}

Mutex* LinuxProcessByteStream::getProcessMutex(uint32_t processId)
{
  s_mutex.lock();

  if(s_processInfo == NULL)
    s_processInfo = new mct::closed_hash_map<uint32_t, Mutex*>();

  auto info = s_processInfo->find(processId);

  // Process hasn't been used yet, create a mutex for it
  if(info == s_processInfo->cend())
  {
    Mutex* newMutex = new Mutex();
    auto i = s_processInfo->insert(std::make_pair(processId, newMutex));

    if(!i.second)
    {
      delete newMutex;
      throw std::logic_error("failed to insert mutex into hash map");
    }

    info = i.first;
  }

  s_mutex.unlock();

  return info->second;
}

void LinuxProcessByteStream::removeProcessMutex(uint32_t processId)
{
  s_mutex.lock();

  auto info = s_processInfo->find(processId);

  if(info != s_processInfo->cend())
  {
    try
    {
      info->second->lock(0);
      delete info->second;
      s_processInfo->erase(info);
    }
    catch(std::runtime_error& ex)
    {
      // Mutex is being used by another thread, leave it alone
    }
  }

  s_mutex.unlock();
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
  return m_pipeIn->getHandle();;
}

void LinuxProcessByteStream::send(const void* buffer, uint32_t size)
{
  m_pipeOut->send(buffer, size);
}

uint32_t LinuxProcessByteStream::receive(void* buffer, uint32_t size)
{
  return m_pipeIn->receive(buffer, size);
}
