#ifndef _MESSAGESTREAM_H
#define _MESSAGESTREAM_H

#include "LetheTypes.h"
#include "WaitObject.h"

namespace lethe
{
  /**
   * The MessageStream class is an interface for all messaging objects that
   *  operate in a message-based fashion, such as UDP.  Send and receive is
   *  meant to be done in-place (zero-copy).  Any object acting as a
   *  MessageStream must implement the following functions:
   *
   * allocate() - allocates a buffer of the requested size, that data may be
   *   written to by a user.  If this buffer cannot be allocated, a
   *   std::out_of_memory exception is thrown.
   *
   * send() - sends an allocated buffer to the remote side
   *
   * receive() - returns the next buffer in the receive queue
   *
   * release() - releases a received buffer, so more data may be sent from the
   *   remote side, may also be used to release an allocated but unsent buffer
   *
   * size() - returns the usable size of the buffer given as the parameter,
   *   corresponding to the size originally allocated
   *
   * operator WaitObject&() - returns a reference to a WaitObject that will be
   *   triggered whenever there is data to receive
   *
   * getHandle() - returns the handle corresponding to the WaitObject that will
   *   be triggered whenever there is data to receive.
   */
  class MessageStream
  {
  public:
    MessageStream();
    virtual ~MessageStream();

    virtual void* allocate(uint32_t) = 0;
    virtual void send(void*) = 0;
    virtual void* receive() = 0;
    virtual void release(void*) = 0;

    virtual uint32_t size(void*) = 0;

    // A stream shall be usable as a wait object
    virtual operator WaitObject&() = 0;
    virtual Handle getHandle() const = 0;
  };
}

#endif
