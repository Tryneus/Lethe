#ifndef _WINDOWSPROCESSMESSAGESTREAM_H
#define _WINDOWSPROCESSMESSAGESTREAM_H

#include "Abstraction.h"

class WindowsProcessMessageStream
{
public:
  WindowsProcessMessageStream();
  ~WindowsProcessMessageStream();

  operator WaitObject&();
  Handle getHandle() const;

  void* allocate(uint32_t size);
  void send(void* buffer);
  void* receive();
  void release(void* buffer);
};

#endif
