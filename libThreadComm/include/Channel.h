#ifndef _THREADCOMM_CHANNEL_H
#define _THREADCOMM_CHANNEL_H

#include "Header.h"
#include "Abstraction.h"

namespace ThreadComm
{

  class Channel : public WaitObject
  {
  public:
    Channel(Header& in,
            Header& out);
    ~Channel();

    void* allocate(uint32_t size);
    void  send(void* msg);
    void* receive();
    void  release(void* msg);

    // TODO: implement postWaitCallback to notify Header that there is a message ready

  private:
    Header& m_in;
    Header& m_out;
  };

}

#endif
