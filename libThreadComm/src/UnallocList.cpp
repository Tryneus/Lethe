#include "UnallocList.h"
#include "Abstraction.h"

using namespace ThreadComm;

UnallocList::UnallocList(void* firstMessage, void* bufferEnd) :
  List(firstMessage),
  m_bufferEnd(bufferEnd)
{
  // Do nothing
}

void UnallocList::unallocate(Message* message)
{
  Message* prevMessage = message->getLastOnStack();
  message->setState(Message::Free);

  // Check if we can merge with the previous buffer in memory
  if(prevMessage != NULL &&
    prevMessage->getState() == Message::Free)
  {
    prevMessage->setSize(prevMessage->getSize() + message->getSize());
    remove(prevMessage);
    message = prevMessage;
  }

  Message* nextMessage = message->getNextOnStack();

  // Check if we can merge with the next buffer in memory
  if(nextMessage < m_bufferEnd &&
    nextMessage->getState() == Message::Free)
  {
    remove(nextMessage);  
    message->setSize(nextMessage->getSize() + message->getSize());
  }

  message->getNextOnStack()->setLastOnStack(message);

  pushFront(message);
}

Message* UnallocList::allocate(uint32_t size)
{
  Message* message = m_front;

  while(message != NULL && message->getSize() < size)
  {
    message = message->getNext();
  }

  if(message != NULL)
  {
    remove(message);

    Message* extra = message->split(size);
    if(extra != NULL) pushFront(extra);

    message->setState(Message::Alloc);
  }

  return message;
}

void UnallocList::remove(Message* message)
{
  if(message->getPrev() != NULL)
    message->getPrev()->setNext(message->getNext());

  if(message->getNext() != NULL)
    message->getNext()->setPrev(message->getPrev());

  if(m_front == message)
    m_front = message->getNext();

  if(m_back == message)
    m_back = message->getPrev();

  message->setPrev(NULL);
  message->setNext(NULL);
}
