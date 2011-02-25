#include "MessageStream/ThreadMessageHeader.h"
#include "LetheException.h"

using namespace lethe;

ThreadMessageHeader::ThreadMessageHeader(uint32_t size, Semaphore& semaphore) :
  m_size(size),
  m_semaphore(semaphore),
  m_dataArea(new char[size]),
  m_receiveList(m_dataArea),
  m_releaseList(m_dataArea + sizeof(ThreadMessage)),
  m_unallocList(m_dataArea + 2 * sizeof(ThreadMessage), &m_dataArea[size])
{
  // Initialize buffers
  ThreadMessage* firstMessage = new (m_dataArea)
    ThreadMessage(this, sizeof(ThreadMessage), ThreadMessage::Nil);

  ThreadMessage* secondMessage = new (m_dataArea + sizeof(ThreadMessage))
    ThreadMessage(this, sizeof(ThreadMessage), ThreadMessage::Pend);

  ThreadMessage* thirdMessage = new (m_dataArea + 2 * sizeof(ThreadMessage))
    ThreadMessage(this, m_size - 2 * sizeof(ThreadMessage), ThreadMessage::Free);

  secondMessage->setLastOnStack(firstMessage);
  thirdMessage->setLastOnStack(secondMessage);
}

ThreadMessageHeader::~ThreadMessageHeader()
{
  delete [] m_dataArea;
}

void* ThreadMessageHeader::getEndPtr()
{
  return (m_dataArea + m_size);
}

Handle ThreadMessageHeader::getHandle() const
{
  return m_semaphore.getHandle();
}

ThreadMessage& ThreadMessageHeader::allocate(uint32_t size)
{
  ThreadMessage* message = m_releaseList.pop();

  while(message != NULL)
  {
    m_unallocList.unallocate(message);
    message = m_releaseList.pop();
  }

  return m_unallocList.allocate(size);
}

void ThreadMessageHeader::send(ThreadMessage& message)
{
  message.setState(ThreadMessage::Sent);
  m_receiveList.pushBack(message);

  m_semaphore.unlock(1);
}

ThreadMessage& ThreadMessageHeader::receive()
{
  ThreadMessage* extraMessage;
  ThreadMessage* message = m_receiveList.receive(extraMessage);

  if(extraMessage != NULL)
    m_releaseList.pushBack(*extraMessage);

  if(message == NULL)
    throw std::logic_error("nothing to receive");

  return *message;
}

bool ThreadMessageHeader::release(ThreadMessage& message)
{
  if(reinterpret_cast<void*>(&message) < reinterpret_cast<void*>(m_dataArea) ||
    reinterpret_cast<void*>(&message) > reinterpret_cast<void*>(m_dataArea + m_size))
    return false; // Message didn't belong to this side, but it might belong to the other side

  switch(message.getState())
  {
  case ThreadMessage::Alloc:
    m_unallocList.unallocate(&message);
    break;
  case ThreadMessage::Recv:
    message.setState(ThreadMessage::Pend);
    m_releaseList.pushBack(message);
    break;
  case ThreadMessage::Nil:
    message.setState(ThreadMessage::Pend);
    break;
  default:
    throw std::invalid_argument("buffer in the wrong state");
  }

  return true;
}
