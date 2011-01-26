#ifndef _THREADCOMM_UNALLOCLIST_H
#define _THREADCOMM_UNALLOCLIST_H

#include "Abstraction.h"
#include "List.h"
#include "Message.h"

namespace ThreadComm
{

  class UnallocList : public List
  {
  public:
    UnallocList(void* firstMessage, void* bufferEnd);

    void unallocate(Message* message);
    Message& allocate(uint32_t size);

  private:
    void remove(Message& message);

    void* m_bufferEnd;
  };

}

#endif
