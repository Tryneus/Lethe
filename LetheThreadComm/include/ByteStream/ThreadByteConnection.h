#ifndef _THREADBYTECONNECTION_H
#define _THREADBYTECONNECTION_H

#include "Lethe.h"
#include "ByteStream/ThreadByteStream.h"

namespace lethe
{
  class ThreadByteConnection
  {
  public:
    ThreadByteConnection();
    ~ThreadByteConnection();

    ByteStream& getStreamA();
    ByteStream& getStreamB();

  private:
    Pipe m_pipeAtoB;
    Pipe m_pipeBtoA;
    ThreadByteStream m_streamA;
    ThreadByteStream m_streamB;
  };
}

#endif
