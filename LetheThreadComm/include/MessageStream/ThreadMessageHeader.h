#ifndef _THREADMESSAGEHEADER_H
#define _THREADMESSAGEHEADER_H

#include "MessageStream/ThreadMessageList.h"
#include "MessageStream/ThreadMessageUnallocList.h"
#include "MessageStream/ThreadMessageReceiveList.h"
#include "MessageStream/ThreadMessage.h"
#include "Lethe.h"

namespace lethe
{
  class ThreadMessageHeader
  {
  public:
    ThreadMessageHeader(uint32_t size, Semaphore& semaphore);
    ~ThreadMessageHeader();

    ThreadMessage& allocate(uint32_t size);
    void send(ThreadMessage& msg);
    ThreadMessage& receive();
    bool release(ThreadMessage& msg);

    void* getEndPtr();
    Handle getHandle() const;

  private:
    uint32_t m_size;
    Semaphore& m_semaphore;

    char* m_dataArea;

    ThreadMessageReceiveList m_receiveList;
    ThreadMessageList m_releaseList;
    ThreadMessageUnallocList m_unallocList;
  };
}

#endif
