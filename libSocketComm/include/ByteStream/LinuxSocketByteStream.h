#ifndef _LINUXSOCKETBYTESTREAM_H
#define _LINUXSOCKETBYTESTREAM_H

#include "Abstraction.h"

class LinuxSocketByteStream
{
public:
  LinuxSocketByteStream();
  ~LinuxSocketByteStream();

  operator WaitObject&();
  Handle getHandle() const;

  void send(void* buffer, uint32_t size);
  uint32_t receive(void* buffer, uint32_t size);
};

#endif
