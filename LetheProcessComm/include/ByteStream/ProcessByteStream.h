#ifndef _PROCESSBYTESTREAM_H
#define _PROCESSBYTESTREAM_H

#include "Lethe.h"
#include <cstdatomic>

namespace lethe
{
  class ProcessByteStream : public ByteStream
  {
  public:
    ProcessByteStream(uint32_t processId, uint32_t timeout);
    ProcessByteStream(ByteStream& stream, uint32_t timeout);
    ~ProcessByteStream();

    operator WaitObject&();
    Handle getHandle() const;

    void send(const void* buffer, uint32_t size);
    uint32_t receive(void* buffer, uint32_t size);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    ProcessByteStream(const ProcessByteStream&);
    ProcessByteStream& operator = (const ProcessByteStream&);

    Pipe* m_pipeIn;
    Pipe* m_pipeOut;
  };
}

#endif
