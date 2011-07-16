#include "Lethe.h"
#include "LetheInternal.h"
#include "MessageStream/LinuxProcessMessageStream.h"
#include "LinuxHandleTransfer.h"
#include <sstream>

using namespace lethe;

const std::string LinuxProcessMessageStream::s_nameBase("lethe-ipc-messagestream-");
std::atomic<uint32_t> LinuxProcessMessageStream::s_nextId(0);

LinuxProcessMessageStream::LinuxProcessMessageStream(ByteStream& stream,
                                                     uint32_t outgoingSize,
                                                     uint32_t timeout) :
  m_semaphoreIn(NULL),
  m_semaphoreOut(NULL),
  m_shmIn(NULL),
  m_shmOut(NULL),
  m_headerIn(NULL),
  m_headerOut(NULL)
{
  uint32_t endTime = getEndTime(timeout);
  char done = '\0';

  outgoingSize = checkSize(outgoingSize);

  try
  {
    // Open local->remote files
    m_shmOut = new SharedMemory(generateShmName(), outgoingSize);
    m_semaphoreOut = new Semaphore(0xFFFFFFFF, 0);

    // Get remote->local files
    doSetup(stream, endTime);

    // Complete synchronization
    stream.send(&done, 1);
    if(WaitForObject(stream, getTimeout(endTime)) != WaitSuccess)
      throw std::runtime_error("message stream constructor did not receive done indication");

    stream.receive(&done, 1);
    if(done != '\0')
      throw std::runtime_error("message stream constructor received incorrect data when waiting for done indication");
  }
  catch(...)
  {
    shutdown();
    throw;
  }

  m_headerIn = reinterpret_cast<ProcessMessageHeader*>(m_shmIn->begin());
  m_headerOut = new (m_shmOut->begin()) ProcessMessageHeader(m_shmOut->size());
}

LinuxProcessMessageStream::~LinuxProcessMessageStream()
{
  shutdown();
}

uint32_t LinuxProcessMessageStream::checkSize(uint32_t size)
{
  if(size < s_minSize)
    throw std::invalid_argument("size too small for communication");

  if(size > s_maxSize)
    size = s_maxSize;

  return size;
}

void LinuxProcessMessageStream::shutdown()
{
  m_headerOut->~ProcessMessageHeader();

  delete m_shmIn;
  delete m_shmOut;
  delete m_semaphoreIn;
  delete m_semaphoreOut;

  m_shmIn = NULL;
  m_shmOut = NULL;
  m_semaphoreIn = NULL;
  m_semaphoreOut = NULL;
}

const std::string LinuxProcessMessageStream::generateShmName()
{
  std::stringstream name;
  name << s_nameBase << getProcessId() << "-" << s_nextId.fetch_add(1);
  return name.str();
}

void LinuxProcessMessageStream::doSetup(ByteStream& stream,
                                        uint32_t endTime)
{
  std::string receivedString;
  char receivedChar;

  stream.send(m_shmOut->name().c_str(), m_shmOut->name().length() + 1);

  while(true)
  {
    if(WaitForObject(stream, getTimeout(endTime)) != WaitSuccess)
      throw std::runtime_error("message stream did not receive complete info: '" + receivedString + "'");

    stream.receive(&receivedChar, 1);

    if(receivedChar == '\0')
      break;

    receivedString.push_back(receivedChar);
  }

  // Format should be <shared memory filename>
  m_shmIn = new SharedMemory(receivedString, 0);

  LinuxHandleTransfer transfer(stream, getTimeout(endTime));

  transfer.sendSemaphore(*m_semaphoreOut);
  m_semaphoreIn = transfer.recvSemaphore(getTimeout(endTime));
}

LinuxProcessMessageStream::operator WaitObject&()
{
  return *m_semaphoreIn;
}

Handle LinuxProcessMessageStream::getHandle() const
{
  return m_semaphoreIn->getHandle();
}

void* LinuxProcessMessageStream::allocate(uint32_t size)
{
  return m_headerOut->allocate(size);
}

void LinuxProcessMessageStream::send(void* buffer)
{
  m_headerOut->send(buffer);
  m_semaphoreOut->unlock(1);
}

void* LinuxProcessMessageStream::receive()
{
  return m_headerIn->receive();
}

void LinuxProcessMessageStream::release(void* buffer)
{
  if(!m_headerIn->release(buffer) && !m_headerOut->release(buffer))
    throw std::invalid_argument("ProcessMessageStream::release buffer");
}