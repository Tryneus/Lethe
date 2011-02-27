#ifndef _PROCESSMESSAGEHEADER_H
#define _PROCESSMESSAGEHEADER_H

#include "MessageStream/ProcessMessageList.h"
#include "MessageStream/ProcessMessageUnallocList.h"
#include "MessageStream/ProcessMessageReceiveList.h"
#include "MessageStream/ProcessMessage.h"
#include "Lethe.h"

namespace lethe
{
  class ProcessMessageHeader
  {
  public:
    ProcessMessageHeader(uint32_t size);
    ~ProcessMessageHeader();

    void* allocate(uint32_t size);
    void send(void* msg);
    void* receive();
    bool release(void* msg);

    uint32_t getSize() const;

  private:
    static const uint32_t s_firstBufferOffset;
    static const uint32_t s_secondBufferOffset;
    static const uint32_t s_thirdBufferOffset;

    uint32_t m_size;
    ProcessMessageReceiveList m_receiveList;
    ProcessMessageList m_releaseList;
    ProcessMessageUnallocList m_unallocList;
  };
}

#endif
