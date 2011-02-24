#ifndef _THREADMESSAGERECEIVELIST_H
#define _THREADMESSAGERECEIVELIST_H

#include "MessageStream/ThreadMessageList.h"

namespace comm
{
  class ThreadMessageReceiveList : public ThreadMessageList
  {
  public:
    ThreadMessageReceiveList(void* firstMessage);

    ThreadMessage* receive(ThreadMessage*& extraMessage);
  };
}

#endif
