#include "MessageStream/ThreadMessageList.h"
#include "Abstraction.h"

using namespace comm;

ThreadMessageList::ThreadMessageList(void* firstMessage)
{
  ThreadMessage* message = reinterpret_cast<ThreadMessage*>(firstMessage);

  m_front = message;
  m_back = message;
}

void ThreadMessageList::pushBack(ThreadMessage& message)
{
  message.setNext(NULL);
  message.setPrev(m_back);
  m_back = &message;
  message.getPrev()->setNext(&message);
}

void ThreadMessageList::pushFront(ThreadMessage& message)
{
  message.setNext(m_front);
  message.setPrev(NULL);

  if(m_front != NULL)
    m_front->setPrev(&message);

  if(m_back == NULL) // Probably not necessary to do this
    m_back = &message;

  m_front = &message;
}

ThreadMessage* ThreadMessageList::pop()
{
  ThreadMessage* message = NULL;

  if(m_front->getNext() != NULL)
  {
    message = m_front;
    m_front = m_front->getNext();
    m_front->setPrev(NULL);
  }

  return message;
}
