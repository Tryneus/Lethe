#ifndef _LINUXPROCESSMESSAGESTREAM_H
#define _LINUXPROCESSMESSAGESTREAM_H

#include "Abstraction.h"

class LinuxProcessMessageStream
{
public:
  LinuxProcessMessageStream();
  ~LinuxProcessMessageStream();

  operator WaitObject&();
  Handle getHandle() const;

  void* allocate(uint32_t size);
  void send(void* buffer);
  void* receive();
  void release(void* buffer);
};

#endif
