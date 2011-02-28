#ifndef _PROCESSMESSAGEUNALLOCLIST_H
#define _PROCESSMESSAGEUNALLOCLIST_H

#include "Lethe.h"
#include "MessageStream/ProcessMessageList.h"
#include "MessageStream/ProcessMessage.h"

namespace lethe
{
  class ProcessMessageUnallocList : public ProcessMessageList
  {
  public:
    ProcessMessageUnallocList(uint32_t offset, uint32_t firstMessage, uint32_t size);

    void unallocate(ProcessMessage* message);
    ProcessMessage* allocate(uint32_t size);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    ProcessMessageUnallocList(const ProcessMessageUnallocList&);
    ProcessMessageUnallocList& operator = (const ProcessMessageUnallocList&);

    void remove(ProcessMessage* message);

    uint32_t m_size;
  };
}

#endif
