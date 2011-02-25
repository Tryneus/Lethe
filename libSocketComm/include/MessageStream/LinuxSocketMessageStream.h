#ifndef _LINUXSOCKETMESSAGESTREAM_H
#define _LINUXSOCKETMESSAGESTREAM_H

#include "Abstraction.h"

class LinuxSocketMessageStream
{
public:
  LinuxSocketMessageStream();
  ~LinuxSocketMessageStream();

  operator WaitObject&();
  Handle getHandle() const;

  void* allocate(uint32_t size);
  void send(void* buffer);
  void* receive();
  void release(void* buffer);
};

#endif