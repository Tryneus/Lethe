#include "Lethe.h"
#include "MessageStream/WindowsSocketMessageStream.h"

using namespace lethe;

WindowsSocketMessageStream::WindowsSocketMessageStream()
{

}

WindowsSocketMessageStream::~WindowsSocketMessageStream()
{

}

WindowsSocketMessageStream::operator WaitObject&()
{
  return *((WaitObject*)NULL);
}

Handle WindowsSocketMessageStream::getHandle() const
{
  return INVALID_HANDLE_VALUE;
}

void* WindowsSocketMessageStream::allocate(uint32_t size GCC_UNUSED)
{
  return NULL;
}

void WindowsSocketMessageStream::send(void* buffer GCC_UNUSED)
{

}

void* WindowsSocketMessageStream::receive()
{
  return NULL;
}

void WindowsSocketMessageStream::release(void* buffer GCC_UNUSED)
{

}
