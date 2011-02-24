#include "Abstraction.h"
#include "ByteStream/LinuxSocketByteStream.h"

LinuxSocketByteStream::LinuxSocketByteStream()
{

}

LinuxSocketByteStream::~LinuxSocketByteStream()
{

}

LinuxSocketByteStream::operator WaitObject&()
{
  return *((WaitObject*)NULL);
}

Handle LinuxSocketByteStream::getHandle() const
{
  return INVALID_HANDLE_VALUE;
}

void LinuxSocketByteStream::send(void* buffer GCC_UNUSED, uint32_t size GCC_UNUSED)
{

}

uint32_t LinuxSocketByteStream::receive(void* buffer GCC_UNUSED, uint32_t size GCC_UNUSED)
{
  return 0;
}
