#ifndef _BYTESTREAM_H
#define _BYTESTREAM_H

#include "LetheTypes.h"
#include "WaitObject.h"

namespace lethe
{
  /**
   * The ByteStream class is an interface for all messaging objects that operate
   *  on a stream of bytes, such as a pipe or TCP connection.  Any object acting
   *  as a ByteStream must implement the following functions:
   *
   * send() - copies the given buffer data into the object, or, if the object is
   *   completely full, will throw a std::out_of_memory exception until space is
   *   available.
   *
   * receive() - Receives data from the stream and copies it into the provided
   *   buffer, and returns the number of bytes received.
   */
  class ByteStream : public WaitObject
  {
  public:
    ByteStream(Handle handle);
    virtual ~ByteStream();

    virtual void send(const void*, uint32_t) = 0;
    virtual uint32_t receive(void*, uint32_t) = 0;
    virtual bool flush(uint32_t) = 0;
  };
}

#endif
