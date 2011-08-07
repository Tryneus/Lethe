#include "Lethe.h"
#include "LetheInternal.h"
#include "WindowsHandleTransfer.h"
#include <sstream>

using namespace lethe;

std::string BaseHandleTransfer::s_syncString("lethe-handle-transfer");

BaseHandleTransfer::BaseHandleTransfer(ByteStream& stream,
                                       uint32_t timeout) :
  m_stream(&stream),
  m_endTime(getEndTime(timeout)),
  m_createdStream(false)
{
  synchronize();
}

BaseHandleTransfer(uint32_t remoteProcessId,
                   uint32_t timeout) :
  m_stream(NULL),
  m_endTime(getEndTime(timeout)),
  m_createdStream(true)
{
  acquireProcessLock(remoteProcessId);

  pipeInName << remoteProcessId << "-to-" << getProcessId();
  pipeOutName << getProcessId() << "-to-" << remoteProcessId;

  m_byteStream = new Pipe(pipeInName.str(), true, pipeOutName.str(), true);

  try
  {
    synchronize();
  }
  catch(...)
  {
    delete m_byteStream;
    throw;
  }
}

BaseHandleTransfer::~BaseHandleTransfer()
{
  if(m_createdStream)
  {
    delete m_byteStream;
    releaseProcessLock(remoteProcessId);
  }
}

void BaseHandleTransfer::acquireProcessLock(uint32_t processId)
{
  s_mutex.lock();

  if(s_processInfo == NULL)
    s_processInfo = new mct::closed_hash_map<uint32_t, Mutex*>();

  mct::closed_hash_map<uint32_t, Mutex*>::iterator info = s_processInfo->find(processId);

  // Process hasn't been used yet, create a mutex for it
  if(info == s_processInfo->cend())
  {
    Mutex* newMutex = new Mutex(true);
    auto i = s_processInfo->insert(std::make_pair(processId, newMutex));

    if(!i.second)
    {
      delete newMutex;
      throw std::logic_error("HandleTransfer failed to insert mutex into hash map");
    }

    info = i.first;
  }

  s_mutex.unlock();
}

void BaseHandleTransfer::removeProcessLock(uint32_t processId)
{
  s_mutex.lock();

  mct::closed_hash_map<uint32_t, Mutex*>::iterator info = s_processInfo->find(processId);

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

void BaseHandleTransfer::synchronize()
{
  char response[strlen(s_syncString)]; // TODO: a more unique sync string

  m_byteStream->send(s_syncString, sizeof(s_syncString));

  if(WaitForObject(*m_byteStream, getTimeout(m_endTime)) != WaitSuccess)
    throw std::runtime_error("HandleTransfer timed out waiting for done indication");

  if(m_byteStream->receive(response, ) != 1 || done != '\0')
    throw std::logic_error("HandleTransfer received unexpected data when synchronizing");
}