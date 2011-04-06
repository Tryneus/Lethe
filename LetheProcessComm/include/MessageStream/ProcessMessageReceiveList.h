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

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    ProcessMessageReceiveList(const ProcessMessageReceiveList&);
    ProcessMessageReceiveList& operator = (const ProcessMessageReceiveList&);
  };
}

#endif
