#ifndef _THREADCOMM_LIST_H
#define _THREADCOMM_LIST_H

#include "Message.h"

namespace ThreadComm
{

  class List
  {
  public:
    List(void* firstMessage);

    void pushBack(Message& message);
    void pushFront(Message& message);
    Message* pop();

  protected:
    Message* m_front;
    Message* m_back;
  };

}

#endif