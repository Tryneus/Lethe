#ifndef _THREADBYTESTREAM_H
#define _THREADBYTESTREAM_H

#include "Lethe.h"

namespace lethe
{
  class ThreadByteStream : public ByteStream
  {
  public:
    ThreadByteStream(Pipe& pipeIn, Pipe& pipeOut);
    ~ThreadByteStream();

    operator WaitObject&();
    Handle getHandle() const;

    void send(void* buffer, uint32_t size);
    uint32_t receive(void* buffer, uint32_t size);

  private:
    Pipe& m_pipeIn;
    Pipe& m_pipeOut;
  };
}

#endif
