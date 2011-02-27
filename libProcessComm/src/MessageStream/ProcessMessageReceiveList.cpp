#include "MessageStream/ProcessMessageReceiveList.h"
#include "Lethe.h"

using namespace lethe;

ProcessMessageReceiveList::ProcessMessageReceiveList(uint32_t offset, uint32_t message) :
  ProcessMessageList(offset, message)
{
  // Do nothing
}

ProcessMessage* ProcessMessageReceiveList::receive(ProcessMessage*& extraMessage)
{
  extraMessage = NULL;
  ProcessMessage* message = getMessage(m_front);

  if(message->getNext() != 0)
  {
    if(message->getState() != ProcessMessage::Sent)
    {
      extraMessage = pop();
      message = getMessage(m_front);

      if(message->getNext() != 0)
      {
        pop();
        message->setState(ProcessMessage::Recv);
      }
      else
        message->setState(ProcessMessage::Nil);
    }
    else
    {
      pop();
      message->setState(ProcessMessage::Recv);
    }
  }
  else if(message->getState() != ProcessMessage::Sent)
  {
    message = NULL;
  }
  else
  {
    message->setState(ProcessMessage::Nil);
  }

  return message;
}
