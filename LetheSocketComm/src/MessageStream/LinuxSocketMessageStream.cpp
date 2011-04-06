#include "Lethe.h"
#include "LetheInternal.h"
#include "MessageStream/LinuxSocketMessageStream.h"

using namespace lethe;

LinuxSocketMessageStream::LinuxSocketMessageStream()
{

}

LinuxSocketMessageStream::~LinuxSocketMessageStream()
{

}

LinuxSocketMessageStream::operator WaitObject&()
{
  return *((WaitObject*)NULL);
}

Handle LinuxSocketMessageStream::getHandle() const
{
  return INVALID_HANDLE_VALUE;
}

void* LinuxSocketMessageStream::allocate(uint32_t size GCC_UNUSED)
{
  return NULL;
}

void LinuxSocketMessageStream::send(void* buffer GCC_UNUSED)
{

}

void* LinuxSocketMessageStream::receive()
{
  return NULL;
}

void LinuxSocketMessageStream::release(void* buffer GCC_UNUSED)
{

}
