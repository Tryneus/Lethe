#include "MessageStream/ThreadMessageUnallocList.h"
#include "AbstractionException.h"

using namespace comm;

ThreadMessageUnallocList::ThreadMessageUnallocList(void* firstMessage, void* bufferEnd) :
  ThreadMessageList(firstMessage),
  m_bufferEnd(bufferEnd)
{
  // Do nothing
}

void ThreadMessageUnallocList::unallocate(ThreadMessage* message)
{
  ThreadMessage* prevMessage = message->getLastOnStack();
  message->setState(ThreadMessage::Free);

  // Check if we can merge with the previous buffer in memory
  if(prevMessage != NULL &&
     prevMessage->getState() == ThreadMessage::Free)
  {
    prevMessage->setSize(prevMessage->getSize() + message->getSize());
    remove(*prevMessage);
    message = prevMessage;
  }

  ThreadMessage& nextMessage = message->getNextOnStack();

  // Check if we can merge with the next buffer in memory
  if(reinterpret_cast<void*>(&nextMessage) != m_bufferEnd)
  {
    if(nextMessage.getState() == ThreadMessage::Free)
    {
      remove(nextMessage);
      message->setSize(nextMessage.getSize() + message->getSize());

      if(&message->getNextOnStack() != m_bufferEnd)
        message->getNextOnStack().setLastOnStack(message);
    }
    else
      message->getNextOnStack().setLastOnStack(message);
  }

  pushFront(*message);
}

ThreadMessage& ThreadMessageUnallocList::allocate(uint32_t size)
{
  ThreadMessage* message = m_front;

  while(message != NULL && message->getSize() < size)
  {
    message = message->getNext();
  }

  if(message == NULL)
    throw std::bad_alloc();

  remove(*message);

  ThreadMessage* extra = message->split(size);
  if(extra != NULL)
    pushFront(*extra);

  message->setState(ThreadMessage::Alloc);

  return *message;
}

void ThreadMessageUnallocList::remove(ThreadMessage& message)
{
  if(message.getPrev() != NULL)
    message.getPrev()->setNext(message.getNext());

  if(message.getNext() != NULL)
    message.getNext()->setPrev(message.getPrev());

  if(m_front == &message)
    m_front = message.getNext();

  if(m_back == &message)
    m_back = message.getPrev();

  message.setPrev(NULL);
  message.setNext(NULL);
}
