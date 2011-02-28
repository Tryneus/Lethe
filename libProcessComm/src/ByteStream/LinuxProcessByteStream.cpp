#include "Lethe.h"
#include "ByteStream/LinuxProcessByteStream.h"

using namespace lethe;

LinuxProcessByteStream::LinuxProcessByteStream(const std::string& pipeIn GCC_UNUSED,
                                               const std::string& pipeOut GCC_UNUSED,
                                               uint32_t timeout GCC_UNUSED)
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
