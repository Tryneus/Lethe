#include "List.h"
#include "Abstraction.h"

using namespace ThreadComm;

List::List(void* firstMessage)
{
  Message* message = reinterpret_cast<Message*>(firstMessage);

  m_front = message;
  m_back = message;
}

void List::pushBack(Message* message)
{
  message->setNext(NULL);
  message->setPrev(m_back);
  m_back = message;
  message->getPrev()->setNext(message);
}

void List::pushFront(Message* message)
{
  message->setNext(m_front);
  message->setPrev(NULL);

  if(m_front != NULL)
    m_front->setPrev(message);

  if(m_back == NULL) // Probably not necessary to do this
    m_back = message;

  m_front = message;
}

Message* List::pop()
{
  Message* message = NULL;

  if(m_front->getNext() != NULL)
  {
    message = m_front;
    m_front = m_front->getNext();
    m_front->setPrev(NULL);
  }

  return message;
}
