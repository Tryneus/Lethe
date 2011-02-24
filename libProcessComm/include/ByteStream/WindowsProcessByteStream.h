#ifndef _WINDOWSPROCESSBYTESTREAM_H
#define _WINDOWSPROCESSBYTESTREAM_H

#include "Abstraction.h"

class WindowsProcessByteStream
{
public:
  WindowsProcessByteStream();
  ~WindowsProcessByteStream();

  operator WaitObject&();
  Handle getHandle() const;

  void send(void* buffer, uint32_t size);
  uint32_t receive(void* buffer, uint32_t size);
};

#endif
