#include "Lethe.h"
#include "ByteStream/WindowsSocketByteStream.h"

using namespace lethe;

WindowsSocketByteStream::WindowsSocketByteStream()
{

}

WindowsSocketByteStream::~WindowsSocketByteStream()
{

}

WindowsSocketByteStream::operator WaitObject&()
{
  return *((WaitObject*)NULL);
}

Handle WindowsSocketByteStream::getHandle() const
{
  return INVALID_HANDLE_VALUE;
}

void WindowsSocketByteStream::send(void* buffer GCC_UNUSED, uint32_t size GCC_UNUSED)
{

}

uint32_t WindowsSocketByteStream::receive(void* buffer GCC_UNUSED, uint32_t size GCC_UNUSED)
{
  return 0;
}
