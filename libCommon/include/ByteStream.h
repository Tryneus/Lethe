#ifndef _BYTESTREAM_H
#define _BYTESTREAM_H

#include "AbstractionTypes.h"
#include "WaitObject.h"

class ByteStream
{
public:
  ByteStream();
  virtual ~ByteStream();

  virtual void send(void*, uint32_t) = 0;
  virtual uint32_t receive(void*, uint32_t) = 0;

  // A stream shall be usable as a wait object
  virtual operator WaitObject&() = 0;
  virtual Handle getHandle() const = 0;
};

#endif
