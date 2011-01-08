#ifndef _THREADCOMM_RECEIVELIST_H
#define _THREADCOMM_RECEIVELIST_H

#include "List.h"

namespace ThreadComm
{

  class ReceiveList : public List
  {
  public:
    ReceiveList(void* firstMessage);

    Message* receive(Message*& extraMessage);
  };

}

#endif