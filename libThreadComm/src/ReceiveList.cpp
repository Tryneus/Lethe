#include "ReceiveList.h"
#include "Abstraction.h"

using namespace ThreadComm;

ReceiveList::ReceiveList(void* firstMessage) :
  List(firstMessage)
{
  // Do nothing
}

Message* ReceiveList::receive(Message*& extraMessage)
{
  extraMessage = NULL;
  Message* message = m_front;

  if(message->getNext() != NULL)
  {
    if(message->getState() != Message::Sent)
    {
      extraMessage = pop();
      message = m_front;

      if(message->getNext() != NULL)
      {
        pop();
        message->setState(Message::Recv);
      }
      else
        message->setState(Message::Nil);
    }
    else
    {
      pop();
      message->setState(Message::Recv);
    }
  }
  else if(message->getState() != Message::Sent)
  {
    message = NULL;
  }
  else
  {
    message->setState(Message::Nil);
  }

  return message;
}
