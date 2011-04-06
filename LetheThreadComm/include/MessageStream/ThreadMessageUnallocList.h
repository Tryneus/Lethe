#ifndef _THREADMESSAGEUNALLOCLIST_H
#define _THREADMESSAGEUNALLOCLIST_H

#include "Lethe.h"
#include "MessageStream/ThreadMessageList.h"
#include "MessageStream/ThreadMessage.h"

namespace lethe
{
  class ThreadMessageUnallocList : public ThreadMessageList
  {
  public:
    ThreadMessageUnallocList(void* firstMessage, void* bufferEnd);

    void unallocate(ThreadMessage* message);
    ThreadMessage& allocate(uint32_t size);

  private:
    void remove(ThreadMessage& message);

    void* m_bufferEnd;
  };
}

#endif
