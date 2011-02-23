#ifndef _THREADCOMM_CHANNEL_H
#define _THREADCOMM_CHANNEL_H

#include "Header.h"
#include "Abstraction.h"

namespace ThreadComm
{

  class Channel
  {
  public:
    Channel(Header& in,
            Header& out,
            WaitObject& obj);
    ~Channel();

    operator WaitObject&();
    Handle getHandle();

    void* allocate(uint32_t size);
    void  send(void* msg);
    void* receive();
    void  release(void* msg);

  private:
    Header& m_in;
    Header& m_out;
    WaitObject& m_waitObject;
  };

}

#endif
