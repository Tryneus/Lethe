#ifndef _THREADMESSAGERECEIVELIST_H
#define _THREADMESSAGERECEIVELIST_H

#include "MessageStream/ThreadMessageList.h"

namespace lethe
{
  class ThreadMessageReceiveList : public ThreadMessageList
  {
  public:
    ThreadMessageReceiveList(void* firstMessage);

    ThreadMessage* receive(ThreadMessage*& extraMessage);
  };
}

#endif
