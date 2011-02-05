#include "Header.h"
#include "Exception.h"
#include "Log.h"

#if defined(_WIN32)
#include <new.h>
#endif

using namespace ThreadComm;

Header::Header(uint32_t size) :
  m_size(size),
  m_semaphore(size / sizeof(Message), 0),
  m_dataArea(new char[size]),
  m_receiveList(m_dataArea),
  m_releaseList(m_dataArea + sizeof(Message)),
  m_unallocList(m_dataArea + 2 * sizeof(Message), &m_dataArea[size])
{
  LogInfo("Header initialized with " << (size / sizeof(Message)) << " max messages");
  // Initialize buffers
  Message* firstMessage = new (m_dataArea)
    Message(this, sizeof(Message), Message::Nil);

  Message* secondMessage = new (m_dataArea + sizeof(Message))
    Message(this, sizeof(Message), Message::Pend);

  Message* thirdMessage = new (m_dataArea + 2 * sizeof(Message))
    Message(this, m_size - 2 * sizeof(Message), Message::Free);

  secondMessage->setLastOnStack(firstMessage);
  thirdMessage->setLastOnStack(secondMessage);
}

Header::~Header()
{
  delete [] m_dataArea;
}

void* Header::getEndPtr()
{
  return (m_dataArea + m_size);
}

Handle Header::getHandle()
{
  return m_semaphore.getHandle();
}

Message& Header::allocate(uint32_t size)
{
  Message* message = m_releaseList.pop();

  while(message != NULL)
  {
    m_unallocList.unallocate(message);
    message = m_releaseList.pop();
  }

  return m_unallocList.allocate(size);
}

void Header::send(Message& message)
{
  message.setState(Message::Sent);
  m_receiveList.pushBack(message);

  m_semaphore.unlock(1);
}

Message& Header::receive()
{
  Message* extraMessage;
  Message* message = m_receiveList.receive(extraMessage);

  if(extraMessage != NULL)
    m_releaseList.pushBack(*extraMessage);

  if(message == NULL)
    throw Exception("Receive called with nothing to receive");

  return *message;
}

bool Header::release(Message& message)
{
  if(reinterpret_cast<void*>(&message) < reinterpret_cast<void*>(m_dataArea) ||
    reinterpret_cast<void*>(&message) > reinterpret_cast<void*>(m_dataArea + m_size))
    return false; // Message didn't belong to this side, but it might belong to the other side

  switch(message.getState())
  {
  case Message::Alloc:
    m_unallocList.unallocate(&message);
    break;
  case Message::Recv:
    message.setState(Message::Pend);
    m_releaseList.pushBack(message);
    break;
  case Message::Nil:
    message.setState(Message::Pend);
    break;
  default:
    throw Exception("Attempt to release a message in the wrong state");
  }

  return true;
}
