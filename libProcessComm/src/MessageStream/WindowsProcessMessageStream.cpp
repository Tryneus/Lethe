#include "Lethe.h"
#include "MessageStream/WindowsProcessMessageStream.h"

using namespace lethe;

WindowsProcessMessageStream::WindowsProcessMessageStream(uint32_t remoteProcessId,
                                                         uint32_t outgoingSize)
{

}

WindowsProcessMessageStream::~WindowsProcessMessageStream()
{

}

WindowsProcessMessageStream::operator WaitObject&()
{
  return *((WaitObject*)NULL);
}

Handle WindowsProcessMessageStream::getHandle() const
{
  return INVALID_HANDLE_VALUE;
}

void* WindowsProcessMessageStream::allocate(uint32_t size GCC_UNUSED)
{
  return NULL;
}

void WindowsProcessMessageStream::send(void* buffer GCC_UNUSED)
{

}

void* WindowsProcessMessageStream::receive()
{
  return NULL;
}

void WindowsProcessMessageStream::release(void* buffer GCC_UNUSED)
{

}
