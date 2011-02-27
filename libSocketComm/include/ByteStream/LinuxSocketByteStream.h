#ifndef _LINUXSOCKETBYTESTREAM_H
#define _LINUXSOCKETBYTESTREAM_H

#include "Lethe.h"

namespace lethe
{
  class LinuxSocketByteStream : public ByteStream
  {
  public:
    LinuxSocketByteStream();
    ~LinuxSocketByteStream();

    operator WaitObject&();
    Handle getHandle() const;

    void send(void* buffer, uint32_t size);
    uint32_t receive(void* buffer, uint32_t size);
  };
}

#endif
