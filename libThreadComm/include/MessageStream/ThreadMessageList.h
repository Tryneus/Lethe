#ifndef _THREADMESSAGELIST_H
#define _THREADMESSAGELIST_H

#include "MessageStream/ThreadMessage.h"

namespace comm
{
  class ThreadMessageList
  {
  public:
    ThreadMessageList(void* firstMessage);

    void pushBack(ThreadMessage& message);
    void pushFront(ThreadMessage& message);
    ThreadMessage* pop();

  protected:
    ThreadMessage* m_front;
    ThreadMessage* m_back;
  };
}

#endif
