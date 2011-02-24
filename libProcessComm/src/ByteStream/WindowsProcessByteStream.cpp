#include "Abstraction.h"
#include "ByteStream/WindowsProcessByteStream.h"

WindowsProcessByteStream::WindowsProcessByteStream()
{

}

WindowsProcessByteStream::~WindowsProcessByteStream()
{

}

WindowsProcessByteStream::operator WaitObject&()
{
  return *((WaitObject*)NULL);
}

Handle WindowsProcessByteStream::getHandle() const
{
  return INVALID_HANDLE_VALUE;
}

void WindowsProcessByteStream::send(void* buffer GCC_UNUSED, uint32_t size GCC_UNUSED)
{

}

uint32_t WindowsProcessByteStream::receive(void* buffer GCC_UNUSED, uint32_t size GCC_UNUSED)
{
  return 0;
}
