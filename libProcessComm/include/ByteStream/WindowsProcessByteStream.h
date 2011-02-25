#ifndef _WINDOWSPROCESSBYTESTREAM_H
#define _WINDOWSPROCESSBYTESTREAM_H

#include "Lethe.h"

namespace lethe
{
  class WindowsProcessByteStream
  {
  public:
    WindowsProcessByteStream(uint32_t remoteProcessId);
    ~WindowsProcessByteStream();

    operator WaitObject&();
    Handle getHandle() const;

    void send(void* buffer, uint32_t size);
    uint32_t receive(void* buffer, uint32_t size);
  };
}

#endif
