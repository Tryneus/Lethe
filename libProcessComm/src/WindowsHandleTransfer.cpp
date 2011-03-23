#include "WindowsHandleTransfer.h"
#include <sstream>

using namespace lethe;

WindowsHandleTransfer::WindowsHandleTransfer(ByteStream& stream,
                                             uint32_t timeout) :
  m_stream(stream)
{
  // Synchronize with other side
}

WindowsHandleTransfer::~WindowsHandleTransfer()
{
  // Do nothing
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

void WindowsHandleTransfer::sendSemaphore(const Semaphore& semaphore)
{
  sendInternal(semaphore.m_name, s_semaphoreType);
}

Pipe* WindowsHandleTransfer::recvPipe(uint32_t timeout)
{
  uint32_t endTime = getTime() + timeout;
  std::string pipeInName = recvInternal(s_pipeType, timeout);

  uint32_t currentTime = getTime();
  timeout = ((currentTime < endTime) ? endTime - currentTime : 0);
  std::string pipeOutName = recvInternal(s_pipeType, timeout);

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

Semaphore* WindowsHandleTransfer::recvSemaphore(uint32_t timeout)
{
  return new Semaphore(recvInternal(s_semaphoreType, timeout));
}

void WindowsHandleTransfer::sendInternal(const std::string& name, char type)
{
  uint32_t bufferSize = name.length() + 3 + sizeof(uint32_t);
  char* buffer = new char[bufferSize];

  // Send format: [32-bit length of buffer] [8-bit type of object] [8-bit null] [ASCII name of object] [8-bit null]
  *reinterpret_cast<uint32_t*>(&buffer[0]) = bufferSize;

  buffer[sizeof(uint32_t)] = type;
  buffer[sizeof(uint32_t) + 1] = '\0';

  name.copy(&buffer[sizeof(uint32_t) + 2], std::string::npos);
  buffer[bufferSize - 1] = '\0';

  try
  {
    m_stream.send(buffer, bufferSize);
  }
  catch(...)
  {
    delete [] buffer;
    throw;
  }

  delete [] buffer;
}

const std::string WindowsHandleTransfer::recvInternal(char type, uint32_t timeout)
{
  uint32_t bufferSize;
  
  if(WaitForObject(m_stream, timeout) != WaitSuccess)
    throw std::runtime_error("did not receive handle data from remote process");

  if(m_stream.receive(&bufferSize, sizeof(uint32_t)) != sizeof(uint32_t))
    throw std::runtime_error("failed to receive handle from remote process");

  char* buffer = new char[bufferSize - sizeof(uint32_t)];

  try
  {
    if(m_stream.receive(buffer, bufferSize - sizeof(uint32_t)) != bufferSize - sizeof(uint32_t))
      throw std::runtime_error("failed to complete handle transfer from remote process");

    if(buffer[0] != type)
      throw std::runtime_error("received handle of unexpected type");

    if(buffer[1] != '\0' || buffer[bufferSize - sizeof(uint32_t) - 1] != '\0')
      throw std::logic_error("incorrect data format in handle transfer stream");
  }
  catch(...)
  {
    delete [] buffer;
    throw;
  }

  std::string name(buffer + 2);
  delete [] buffer;
  return name;
}
