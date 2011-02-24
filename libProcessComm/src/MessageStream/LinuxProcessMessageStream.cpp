#include "Abstraction.h"
#include "MessageStream/LinuxProcessMessageStream.h"

LinuxProcessMessageStream::LinuxProcessMessageStream()
{

}

LinuxProcessMessageStream::~LinuxProcessMessageStream()
{

}

LinuxProcessMessageStream::operator WaitObject&()
{
  return *((WaitObject*)NULL);
}

Handle LinuxProcessMessageStream::getHandle() const
{
  return INVALID_HANDLE_VALUE;
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
