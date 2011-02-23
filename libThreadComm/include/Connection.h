#ifndef _THREADCOMM_CONNETION_H
#define _THREADCOMM_CONNETION_H

#include "Abstraction.h"
#include "Channel.h"

namespace ThreadComm
{

  class Connection
  {
  public:
    Connection(uint32_t sizeAtoB, uint32_t sizeBtoA);
    ~Connection();

    static uint32_t checkSize(uint32_t size);

    Channel& getChannelA();
    Channel& getChannelB();

  private:
    static const uint32_t s_minSize = 20 * sizeof(Message);
    static const uint32_t s_maxSize = (1 << 20);

    Semaphore m_semaphoreAtoB;
    Semaphore m_semaphoreBtoA;
    Header m_headerAtoB;
    Header m_headerBtoA;
    Channel m_channelA;
    Channel m_channelB;
  };

}

#endif
