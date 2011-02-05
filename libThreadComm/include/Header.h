#ifndef _THREADCOMM_HEADER_H
#define _THREADCOMM_HEADER_H

#include "List.h"
#include "UnallocList.h"
#include "ReceiveList.h"
#include "Message.h"
#include "Abstraction.h"

namespace ThreadComm
{
  class Header
  {
  public:
    Header(uint32_t size);
    ~Header();

    Message& allocate(uint32_t size);
    void     send(Message& msg);
    Message& receive();
    bool     release(Message& msg);

    void* getEndPtr();
    Handle getHandle();

  private:
    uint32_t m_size;
    Semaphore m_semaphore;

    char* m_dataArea;

    ReceiveList m_receiveList;
    List m_releaseList;
    UnallocList m_unallocList;
  };

}

#endif
