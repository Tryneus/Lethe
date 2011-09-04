#include "Lethe.h"
#include "LetheInternal.h"
#include "TempProcessStream.h"
#include <sstream>
#include <iostream>

using namespace lethe;

const std::string TempProcessStream::s_baseName("lethe-process-pipe-");
std::set<uint32_t> TempProcessStream::s_processInfo;
Mutex TempProcessStream::s_mutex(false);

TempProcessStream::TempProcessStream(uint32_t remoteProcessId) :
  m_stream(NULL),
  m_remoteProcessId(remoteProcessId)
{
  std::stringstream pipeInName;
  std::stringstream pipeOutName;

  // Create the default stream based on the end-points
  acquireProcessLock(m_remoteProcessId);

  pipeInName << m_remoteProcessId << "-to-" << getProcessId();
  pipeOutName << getProcessId() << "-to-" << m_remoteProcessId;

  std::cout << "opening in '" << pipeInName.str() << "' and out '" << pipeOutName.str() << "'" << std::endl;

  try
  {
    // Using the default stream, negotiate a new, unique stream
    m_stream = new Pipe(pipeInName.str(), true, pipeOutName.str(), true);
  }
  catch(...)
  {
    delete m_stream;
    releaseProcessLock(m_remoteProcessId);
    throw;
  }
}

TempProcessStream::~TempProcessStream()
{
  delete m_stream;
  releaseProcessLock(m_remoteProcessId);
}

void TempProcessStream::acquireProcessLock(uint32_t processId)
{
  s_mutex.lock();

  if(s_processInfo.count(processId) == 0)
    s_processInfo.insert(processId);
  else
    throw std::runtime_error("default temporary stream between these two processes is already in use");

  s_mutex.unlock();
}

void TempProcessStream::releaseProcessLock(uint32_t processId)
{
  s_mutex.lock();
  s_processInfo.erase(processId);
  s_mutex.unlock();
}

void TempProcessStream::send(const void* buffer, uint32_t size)
{
  m_stream->send(buffer, size);
}

uint32_t TempProcessStream::receive(void* buffer, uint32_t size)
{
  return m_stream->receive(buffer, size);
}

TempProcessStream::operator WaitObject&()
{
  return *m_stream;
}

Handle TempProcessStream::getHandle() const
{
  return m_stream->getHandle();
}
