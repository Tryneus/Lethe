#include "MessageStream/ProcessMessageUnallocList.h"
#include "LetheException.h"

using namespace lethe;

ProcessMessageUnallocList::ProcessMessageUnallocList(uint32_t offset, uint32_t message, uint32_t size) :
  ProcessMessageList(offset, message),
  m_size(size)
{
  // Do nothing
}

void ProcessMessageUnallocList::unallocate(ProcessMessage* message)
{
  ProcessMessage* prevMessage = getMessage(message->getLastOnStack());
  message->setState(ProcessMessage::Free);

  // Check if we can merge with the previous buffer in memory
  if(prevMessage != NULL &&
     prevMessage->getState() == ProcessMessage::Free)
  {
    prevMessage->setSize(prevMessage->getSize() + message->getSize());
    remove(prevMessage);
    message = prevMessage;
  }

  ProcessMessage* nextMessage = getMessage(message->getNextOnStack());

  // Check if we can merge with the next buffer in memory
  if(nextMessage->getOffset() != m_size)
  {
    if(nextMessage->getState() == ProcessMessage::Free)
    {
      remove(nextMessage);
      message->setSize(nextMessage->getSize() + message->getSize());

      if(message->getNextOnStack() != m_size)
        getMessage(message->getNextOnStack())->setLastOnStack(message->getOffset());
    }
    else
      getMessage(message->getNextOnStack())->setLastOnStack(message->getOffset());
  }

  pushFront(message);
}

ProcessMessage* ProcessMessageUnallocList::allocate(uint32_t size)
{
  if(m_front != 0)
  {
    ProcessMessage* message = getMessage(m_front);

    while(message->getNext() != 0 && message->getSize() < size)
    {
      message = getMessage(message->getNext());
    }

    if(message->getSize() < size)
      throw std::bad_alloc();

    remove(message);

    ProcessMessage* extra = message->split(size);
    if(extra != NULL)
      pushFront(extra);

    message->setState(ProcessMessage::Alloc);

    return message;
  }

  throw std::bad_alloc();
}

void ProcessMessageUnallocList::remove(ProcessMessage* message)
{
  if(message->getPrev() != 0)
    getMessage(message->getPrev())->setNext(message->getNext());

  if(message->getNext() != 0)
    getMessage(message->getNext())->setPrev(message->getPrev());

  if(m_front == message->getOffset())
    m_front = message->getNext();

  if(m_back == message->getOffset())
    m_back = message->getPrev();

  message->setPrev(0);
  message->setNext(0);
}
