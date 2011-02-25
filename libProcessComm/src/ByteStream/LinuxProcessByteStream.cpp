#include "Lethe.h"
#include "ByteStream/LinuxProcessByteStream.h"

using namespace lethe;

LinuxProcessByteStream::LinuxProcessByteStream(uint32_t remoteProcessId GCC_UNUSED)
{

}

LinuxProcessByteStream::~LinuxProcessByteStream()
{

}

LinuxProcessByteStream::operator WaitObject&()
{
  return *((WaitObject*)NULL);
}

Handle LinuxProcessByteStream::getHandle() const
{
  return INVALID_HANDLE_VALUE;
}

void LinuxProcessByteStream::send(void* buffer GCC_UNUSED, uint32_t size GCC_UNUSED)
{

}

uint32_t LinuxProcessByteStream::receive(void* buffer GCC_UNUSED, uint32_t size GCC_UNUSED)
{
  return 0;
}
