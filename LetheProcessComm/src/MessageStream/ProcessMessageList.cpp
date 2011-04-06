#include "MessageStream/ProcessMessageList.h"
#include "Lethe.h"

using namespace lethe;

ProcessMessageList::ProcessMessageList(uint32_t offset, uint32_t message) :
  m_offset(offset),
  m_front(message),
  m_back(message)
{
  // Do nothing
}

ProcessMessage* ProcessMessageList::getMessage(uint32_t offset)
{
  return reinterpret_cast<ProcessMessage*>(reinterpret_cast<uint8_t*>(this) - m_offset + offset);
}

void ProcessMessageList::pushBack(ProcessMessage* message)
{
  message->setNext(0);
  message->setPrev(m_back);
  m_back = message->getOffset();
  getMessage(message->getPrev())->setNext(message->getOffset());
}

void ProcessMessageList::pushFront(ProcessMessage* message)
{
  message->setNext(m_front);
  message->setPrev(0);

  if(m_front != 0)
    getMessage(m_front)->setPrev(message->getOffset());

  if(m_back == 0) // Probably not necessary to do this
    m_back = message->getOffset();

  m_front = message->getOffset();
}

ProcessMessage* ProcessMessageList::pop()
{
  ProcessMessage* message = getMessage(m_front);

  if(message->getNext() != 0)
  {
    m_front = message->getNext();
    getMessage(m_front)->setPrev(0);
  }
  else
    message = NULL;

  return message;
}
