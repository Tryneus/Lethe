#include "Lethe.h"
#include "LetheInternal.h"
#include "WindowsHandleTransfer.h"
#include <sstream>

using namespace lethe;

WindowsHandleTransfer::WindowsHandleTransfer(ByteStream& stream,
                                             uint32_t timeout) :
  m_stream(NULL)
{
  uint64_t endTime = getEndTime(timeout);

  m_stream = negotiateUniqueStream(stream, endTime);

  try
  {
    synchronize(*m_stream, endTime);
  }
  catch(...)
  {
    delete m_stream;
    throw;
  }
}

WindowsHandleTransfer(uint32_t remoteProcessId,
                      uint32_t timeout) :
  m_stream(NULL)
{
  uint64_t endTime = getEndTime(timeout);
  TempProcessStream tempStream(remoteProcessId);

  m_stream = negotiateUniqueStream(tempStream, endTime);

  try
  {
    synchronize(*m_stream, endTime);
  }
  catch(...)
  {
    delete m_stream;
    throw;
  }
}

void WindowsHandleTransfer::synchronize(ByteStream& stream, uint64_t endTime)
{
  char response[s_syncString.length() + 1];

  stream.send(s_syncString.c_str(), s_syncString.length() + 1);

  if(WaitForObject(stream, getTimeout(endTime)) != WaitSuccess)
    throw std::runtime_error("WindowsHandleTransfer timed out waiting for done indication");

  if(stream.receive(response, s_syncString.length() + 1) != s_syncString.length() + 1 ||
     response != s_syncString)
    throw std::logic_error("WindowsHandleTransfer received unexpected data when synchronizing");
}

ByteStream* WindowsHandleTransfer::negotiateUniqueStream(ByteStream& stream, uint64_t endTime)
{
  uint32_t maxNameLength = s_baseName.length() + 22;
  uint32_t receiveLength;
  char response[maxNameLength + 1];
  std::string pipeInName;
  std::stringstream pipeOutName;

  pipeOutName << s_baseName << "-" << getProcessId() << "-" << s_uniqueId.increment();

  stream.write(pipeOutName.str().c_str(), pipeOutName.str().length() + 1);

  if(WaitForObject(stream, getTimeout(endTime)) != WaitSuccess)
    throw std::runtime_error("WindowsHandleTransfer timed out waiting for done indication");

  receiveLength = stream.receive(response, maxNameLength + 1);

  // Throw an error if we don't find the correct format
  if(receiveLength == maxNameLength + 1 ||
     pipeInName[receiveLength - 1] != '\0')
    throw std::logic_error("WindowsHandleTransfer received unexpected data when synchronizing");

  pipeInName.assign(response);

  if(pipeInName.find(s_baseName) != 0)
    throw std::logic_error("WindowsHandleTransfer received unexpected data when synchronizing");

  return new Pipe(pipeOutName.str(), true, pipeInName, true);
}

void WindowsHandleTransfer::sendPipe(const Pipe& pipe)
{
  // WindowsHandleTransfer has private access to Pipe, both handles are sent
  sendInternal(pipe.m_pipeWriteName, s_pipeType);
  sendInternal(pipe.m_pipeReadName, s_pipeType);
}

void WindowsHandleTransfer::sendTimer(const Timer& timer)
{
  sendInternal(timer.m_name, s_timerType);
}

void WindowsHandleTransfer::sendEvent(const Event& event)
{
  sendInternal(event.m_name, s_eventType);
}

void WindowsHandleTransfer::sendMutex(const Mutex& mutex)
{
  sendInternal(mutex.m_name, s_mutexType);
}

void WindowsHandleTransfer::sendSemaphore(const Semaphore& semaphore)
{
  sendInternal(semaphore.m_name, s_semaphoreType);
}

Pipe* WindowsHandleTransfer::recvPipe(uint32_t timeout)
{
  uint32_t endTime = getEndTime(timeout);
  std::string pipeInName = recvInternal(s_pipeType, timeout);
  std::string pipeOutName = recvInternal(s_pipeType, getTimeout(endTime));

  return new Pipe(pipeInName, pipeOutName);
}

Timer* WindowsHandleTransfer::recvTimer(uint32_t timeout)
{
  return new Timer(recvInternal(s_timerType, timeout));
}

Event* WindowsHandleTransfer::recvEvent(uint32_t timeout)
{
  return new Event(recvInternal(s_eventType, timeout));
}

Mutex* WindowsHandleTransfer::recvMutex(uint32_t timeout)
{
  return new Mutex(recvInternal(s_mutexType, timeout));
}

Semaphore* WindowsHandleTransfer::recvSemaphore(uint32_t timeout)
{
  return new Semaphore(recvInternal(s_semaphoreType, timeout));
}

void WindowsHandleTransfer::sendInternal(const std::string& name, char type)
{
  char buffer[name.length() + sizeof(HandleData)];
  HandleData* data = reinterpret_cast<HandleData*>(buffer);

  data->length = name.length() + sizeof(HandleData);
  data->type = type;
  data->null = '\0';
  name.copy(&data->name, std::string::npos);
  data->name[name.length()] = '\0'

  m_stream.send(data, data->length);
}

const std::string WindowsHandleTransfer::recvInternal(char type, uint32_t timeout)
{
  uint32_t bufferSize;
  std::string name;

  if(WaitForObject(m_stream, timeout) != WaitSuccess)
    throw std::runtime_error("did not receive handle data from remote process");

  if(m_stream.receive(&bufferSize, sizeof(uint32_t)) != sizeof(uint32_t))
    throw std::runtime_error("failed to receive handle from remote process");

  char buffer[bufferSize];
  HandleData* data = reinterpret_cast<HandleData*>(buffer);

  // Receive the rest of the data, starting from the type
  if(m_stream.receive(&data->type, bufferSize - sizeof(uint32_t)) != bufferSize - sizeof(uint32_t))
    throw std::runtime_error("failed to complete handle transfer from remote process");

  if(data->type != type)
    throw std::runtime_error("received handle of unexpected type");

  if(data->null != '\0' || data->name[bufferSize - sizeof(HandleData)] != '\0')
    throw std::logic_error("incorrect data format in handle transfer stream");

  name.assign(data.name());
  return name;
}
