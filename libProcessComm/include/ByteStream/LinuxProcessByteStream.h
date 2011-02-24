#ifndef _LINUXPROCESSBYTESTREAM_H
#define _LINUXPROCESSBYTESTREAM_H

#include "Abstraction.h"

class LinuxProcessByteStream
{
public:
  LinuxProcessByteStream();
  ~LinuxProcessByteStream();

  operator WaitObject&();
  Handle getHandle() const;

  void send(void* buffer, uint32_t size);
  uint32_t receive(void* buffer, uint32_t size);
};

#endif
