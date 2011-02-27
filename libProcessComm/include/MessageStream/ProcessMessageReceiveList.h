#ifndef _PROCESSMESSAGERECEIVELIST_H
#define _PROCESSMESSAGERECEIVELIST_H

#include "MessageStream/ProcessMessageList.h"

namespace lethe
{
  class ProcessMessageReceiveList : public ProcessMessageList
  {
  public:
    ProcessMessageReceiveList(uint32_t offset, uint32_t firstMessage);

    ProcessMessage* receive(ProcessMessage*& extraMessage);
  };
}

#endif
