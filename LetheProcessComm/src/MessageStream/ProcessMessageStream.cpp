#define __STDC_LIMIT_MACROS
#include "Lethe.h"
#include "LetheInternal.h"
#include "ProcessComm.h"
#include "MessageStream/ProcessMessageStream.h"
#include <sstream>

using namespace lethe;

const std::string ProcessMessageStream::s_syncString("lethe-process-message-sync");

ProcessMessageStream::ProcessMessageStream(ByteStream& stream,
                                           uint32_t outgoingSize,
                                           uint32_t timeout) :
  MessageStream(INVALID_HANDLE_VALUE),
  m_shmOut(checkSize(outgoingSize)),
  m_headerOut(new (m_shmOut.begin()) ProcessMessageHeader(m_shmOut.size())),
  m_semaphoreOut(UINT32_MAX, 0),
  m_shmIn(NULL),
  m_headerIn(NULL),
  m_semaphoreIn(NULL)
{
  try
  {
    doSetup(stream, getEndTime(timeout));
  }
  catch(...)
  {
    shutdown();
    throw;
  }

  m_headerIn = reinterpret_cast<ProcessMessageHeader*>(m_shmIn->begin());
  setHandle(m_semaphoreIn->getHandle());
}

ProcessMessageStream::ProcessMessageStream(uint32_t remoteProcessId,
                                           uint32_t outgoingSize,
                                           uint32_t timeout) :
  MessageStream(INVALID_HANDLE_VALUE),
  m_shmOut(checkSize(outgoingSize)),
  m_headerOut(new (m_shmOut.begin()) ProcessMessageHeader(m_shmOut.size())),
  m_semaphoreOut(UINT32_MAX, 0),
  m_shmIn(NULL),
  m_headerIn(NULL),
  m_semaphoreIn(NULL)
{
  try
  {
    // Since we don't have a stream to use, create a temporary one
    TempProcessStream stream(remoteProcessId);
    doSetup(stream, getEndTime(timeout));
  }
  catch(...)
  {
    shutdown();
    throw;
  }

  m_headerIn = reinterpret_cast<ProcessMessageHeader*>(m_shmIn->begin());
  setHandle(m_semaphoreIn->getHandle());
}

ProcessMessageStream::~ProcessMessageStream()
{
  shutdown();
}

uint32_t ProcessMessageStream::checkSize(uint32_t size)
{
  if(size < s_minSize)
    throw std::invalid_argument("size too small for communication");

  if(size > s_maxSize)
    size = s_maxSize;

  return size;
}

void ProcessMessageStream::shutdown()
{
  m_headerOut->~ProcessMessageHeader();

  delete m_shmIn;
  delete m_semaphoreIn;

  m_shmIn = NULL;
  m_semaphoreIn = NULL;
}

void ProcessMessageStream::doSetup(ByteStream& stream,
                                   uint64_t endTime)
{
  size_t nameLength = m_shmOut.name().length() + 1;
  char syncBuffer[s_syncString.length() + 1];
  
  // Write the sync string, the length of the outgoing name, then the name itself
  stream.send(s_syncString.c_str(), s_syncString.length() + 1);
  stream.send(&nameLength, sizeof(nameLength));
  stream.send(m_shmOut.name().c_str(), m_shmOut.name().length() + 1);

  // Receive the sync string
  if(WaitForObject(stream, getTimeout(endTime)) != WaitSuccess)
    throw std::runtime_error("failed to synchronize with remote process");

  if(stream.receive(syncBuffer, s_syncString.length() + 1) != s_syncString.length() + 1)
    throw std::runtime_error("unexpected data length during synchronization");

  // Verify the sync string
  if(syncBuffer[s_syncString.length()] != '\0' || std::string(syncBuffer) != s_syncString)
    throw std::runtime_error("unexpected snchronization string");

  // Receive the length of the name
  if(WaitForObject(stream, getTimeout(endTime)) != WaitSuccess)
    throw std::runtime_error("failed to synchronize with remote process");

  if(stream.receive(&nameLength, sizeof(nameLength)) != sizeof(nameLength))
    throw std::runtime_error("unexpected data length during synchronization");

  // Receive the name string
  char nameBuffer[nameLength];
  if(WaitForObject(stream, getTimeout(endTime)) != WaitSuccess)
    throw std::runtime_error("failed to synchronize with remote process");

  if(stream.receive(nameBuffer, nameLength) != nameLength)
    throw std::runtime_error("unexpected data length during synchronization");

  // Verify the sync string
  if(nameBuffer[nameLength - 1] != '\0')
    throw std::runtime_error("unexpected name string during synchronization");

  std::string nameIn(nameBuffer);
  if(nameIn.length() != nameLength - 1)
    throw std::runtime_error("unexpected name string during synchronization");

  m_shmIn = new SharedMemory(nameIn);

  HandleTransfer transfer(stream, getTimeout(endTime));

  transfer.sendSemaphore(m_semaphoreOut);
  m_semaphoreIn = transfer.recvSemaphore(getTimeout(endTime));

  char done = '\0';

  // Complete synchronization
  stream.send(&done, 1);
  if(WaitForObject(stream, getTimeout(endTime)) != WaitSuccess)
    throw std::runtime_error("message stream constructor did not receive done indication");

  if(stream.receive(&done, 1) != 1)
    throw std::runtime_error("message stream did not receive done response");

  if(done != '\0')
    throw std::runtime_error("message stream constructor received incorrect data when waiting for done indication");
}

void* ProcessMessageStream::allocate(uint32_t size)
{
  size += sizeof(ProcessMessage);
  size += sizeof(uint64_t) - (size % sizeof(uint64_t)); // Align along 64-bit boundary

  // Check for integer overflow
  if(size < sizeof(ProcessMessage))
    throw std::bad_alloc();

  return m_headerOut->allocate(size).getDataArea();
}

void ProcessMessageStream::send(void* buffer)
{
  m_headerOut->send(ProcessMessage::getMessage(buffer));
  m_semaphoreOut.unlock(1);
}

void* ProcessMessageStream::receive()
{
  try
  {
    return m_headerIn->receive()->getDataArea();
  }
  catch(std::exception &ex)
  {
    return NULL;
  }
}

void ProcessMessageStream::release(void* buffer)
{
  ProcessMessage* message = ProcessMessage::getMessage(buffer);

  if(!m_headerIn->release(message) && !m_headerOut->release(message))
    throw std::invalid_argument("ProcessMessageStream::release buffer");
}

uint32_t ProcessMessageStream::size(void* buffer)
{
  return ProcessMessage::getMessage(buffer)->getSize();
}
