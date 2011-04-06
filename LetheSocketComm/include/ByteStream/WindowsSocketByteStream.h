#ifndef _WINDOWSSOCKETBYTESTREAM_H
#define _WINDOWSSOCKETBYTESTREAM_H

#include "Lethe.h"

namespace lethe
{
  class WindowsSocketByteStream : public ByteStream
  {
  public:
    WindowsSocketByteStream();
    ~WindowsSocketByteStream();

    operator WaitObject&();
    Handle getHandle() const;

    void send(void* buffer, uint32_t size);
    uint32_t receive(void* buffer, uint32_t size);
  };
}

#endif
