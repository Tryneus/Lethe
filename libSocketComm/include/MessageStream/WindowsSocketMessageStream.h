#ifndef _WINDOWSSOCKETMESSAGESTREAM_H
#define _WINDOWSSOCKETMESSAGESTREAM_H

#include "Lethe.h"

namespace lethe
{
  class WindowsSocketMessageStream
  {
  public:
    WindowsSocketMessageStream();
    ~WindowsSocketMessageStream();

    operator WaitObject&();
    Handle getHandle() const;

    void* allocate(uint32_t size);
    void send(void* buffer);
    void* receive();
    void release(void* buffer);
  };
}

#endif
