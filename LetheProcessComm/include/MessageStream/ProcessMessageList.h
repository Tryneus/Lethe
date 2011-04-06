#ifndef _PROCESSMESSAGELIST_H
#define _PROCESSMESSAGELIST_H

#include "MessageStream/ProcessMessage.h"

namespace lethe
{
  class ProcessMessageList
  {
  public:
    ProcessMessageList(uint32_t offset, uint32_t firstMessage);

    void pushBack(ProcessMessage* message);
    void pushFront(ProcessMessage* message);
    ProcessMessage* pop();

  protected:
    ProcessMessage* getMessage(uint32_t offset);

    uint32_t m_offset;
    uint32_t m_front;
    uint32_t m_back;

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    ProcessMessageList(const ProcessMessageList&);
    ProcessMessageList& operator = (const ProcessMessageList&);
  };
}

#endif
