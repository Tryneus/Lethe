#ifndef _MESSAGESTREAM_H
#define _MESSAGESTREAM_H

#include "LetheTypes.h"
#include "WaitObject.h"

namespace lethe
{
  class MessageStream
  {
  public:
    MessageStream();
    virtual ~MessageStream();

    virtual void* allocate(uint32_t) = 0;
    virtual void send(void*) = 0;
    virtual void* receive() = 0;
    virtual void release(void*) = 0;

    // A stream shall be usable as a wait object
    virtual operator WaitObject&() = 0;
    virtual Handle getHandle() const = 0;
  };
}

#endif
