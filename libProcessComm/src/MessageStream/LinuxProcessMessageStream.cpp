#include "Abstraction.h"
#include "MessageStream/LinuxProcessMessageStream.h"

LinuxProcessMessageStream::LinuxProcessMessageStream(uint32_t remoteProcessId,
                                                     uint32_t outgoingSize GCC_UNUSED) :
  m_inName(getInName(remoteProcessId)),
  m_outName(getOutName(remoteProcessId)),
  m_pipeIn(m_inName),
  m_pipeOut(m_outName),
  m_shmIn(NULL),
  m_shmOut(NULL),
  m_headerIn(NULL),
  m_headerOut(NULL)
{

}

LinuxProcessMessageStream::~LinuxProcessMessageStream()
{

}

const std::string getInName(uint32_t remoteProcessId GCC_UNUSED)
{
  return "";
}

const std::string getOutName(uint32_t remoteProcessId GCC_UNUSED)
{
  return "";
}

LinuxProcessMessageStream::operator WaitObject&()
{
  return m_pipeIn;
}

Handle LinuxProcessMessageStream::getHandle() const
{
  return m_pipeIn.getHandle();
}

void* LinuxProcessMessageStream::allocate(uint32_t size GCC_UNUSED)
{
  return NULL;
}

void LinuxProcessMessageStream::send(void* buffer GCC_UNUSED)
{

}

void* LinuxProcessMessageStream::receive()
{
  return NULL;
}

void LinuxProcessMessageStream::release(void* buffer GCC_UNUSED)
{

}
