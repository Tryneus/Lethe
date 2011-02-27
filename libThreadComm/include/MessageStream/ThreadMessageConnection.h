#ifndef _THREADMESSAGECONNECTION_H
#define _THREADMESSAGECONNECTION_H

#include "Lethe.h"
#include "MessageStream/ThreadMessageStream.h"

namespace lethe
{
  class ThreadMessageConnection
  {
  public:
    ThreadMessageConnection(uint32_t sizeAtoB, uint32_t sizeBtoA);
    ~ThreadMessageConnection();

    MessageStream& getStreamA();
    MessageStream& getStreamB();

  private:
    static uint32_t checkSize(uint32_t size);

    static const uint32_t s_minSize = 20 * sizeof(ThreadMessage) + sizeof(ThreadMessageHeader);
    static const uint32_t s_maxSize = (1 << 25); // Limit: 32 MB

    Semaphore m_semaphoreAtoB;
    Semaphore m_semaphoreBtoA;
    ThreadMessageHeader m_headerAtoB;
    ThreadMessageHeader m_headerBtoA;
    ThreadMessageStream m_streamA;
    ThreadMessageStream m_streamB;
  };
}

#endif
