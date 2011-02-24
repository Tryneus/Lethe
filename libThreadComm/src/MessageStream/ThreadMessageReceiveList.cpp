#include "MessageStream/ThreadMessageReceiveList.h"
#include "Abstraction.h"

using namespace comm;

ThreadMessageReceiveList::ThreadMessageReceiveList(void* firstMessage) :
  ThreadMessageList(firstMessage)
{
  // Do nothing
}

ThreadMessage* ThreadMessageReceiveList::receive(ThreadMessage*& extraMessage)
{
  extraMessage = NULL;
  ThreadMessage* message = m_front;

  if(message->getNext() != NULL)
  {
    if(message->getState() != ThreadMessage::Sent)
    {
      extraMessage = pop();
      message = m_front;

      if(message->getNext() != NULL)
      {
        pop();
        message->setState(ThreadMessage::Recv);
      }
      else
        message->setState(ThreadMessage::Nil);
    }
    else
    {
      pop();
      message->setState(ThreadMessage::Recv);
    }
  }
  else if(message->getState() != ThreadMessage::Sent)
  {
    message = NULL;
  }
  else
  {
    message->setState(ThreadMessage::Nil);
  }

  return message;
}
