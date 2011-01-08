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

    Message* allocate(uint32_t size);
    void     send(Message* msg);
    Message* receive();
    bool     release(Message* msg);

    void* getEndPtr() { return (m_dataArea + m_size); };

  private:
    uint32_t m_size;
    char* m_dataArea;

    ReceiveList m_receiveList;
    List m_releaseList;
    UnallocList m_unallocList;

#if defined(_WIN32)
    Handle m_semaphore;
    bool m_waitingToSend;
  public:
    Handle getHandle() { return m_semaphore; };  

#elif defined(__linux__)
    Handle m_pipeIn;
    Handle m_pipeOut;

  public:
    Handle getHandle() { return m_pipeIn; };
#else
  #error "Unsupported platform"
#endif



  };

}

#endif