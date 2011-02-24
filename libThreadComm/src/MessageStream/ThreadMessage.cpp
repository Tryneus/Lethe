#include "MessageStream/ThreadMessage.h"
#include "MessageStream/ThreadMessageHeader.h"
#include "Abstraction.h"
#include "Log.h"

#define FIRST_MAGIC 0x849D07F3 // Guaranteed to be random, chosen by a fair dice roll
#define SECOND_MAGIC ~FIRST_MAGIC

using namespace comm;

ThreadMessage::ThreadMessage(ThreadMessageHeader* header, uint32_t size, State state) :
  m_header(header),
  m_prev(NULL),
  m_next(NULL),
  m_lastOnStack(NULL),
  m_size(size),
  m_state(state),
  m_magic(FIRST_MAGIC)
{
  getSecondMagic() = SECOND_MAGIC;
}

ThreadMessage* ThreadMessage::split(uint32_t size)
{
  uint32_t extraSize = m_size - size;
  ThreadMessage* extra = NULL;

  if(extraSize > sizeof(ThreadMessage))
  {
    m_size = size;

    extra = new (&getNextOnStack())
      ThreadMessage(m_header, extraSize, ThreadMessage::Free);

    extra->m_lastOnStack = this;
    getSecondMagic() = SECOND_MAGIC;

    if(&extra->getNextOnStack() < (void*)((uint8_t*)m_header->getEndPtr() - sizeof(ThreadMessage)))
    {
      extra->getNextOnStack().setLastOnStack(extra);
    }
  }

  return extra;
}

bool ThreadMessage::overflowCheck()
{
  return (m_magic == FIRST_MAGIC) && (getSecondMagic() == SECOND_MAGIC);
}

ThreadMessageHeader* ThreadMessage::getHeader()
{
  return m_header;
}

ThreadMessage* ThreadMessage::getPrev()
{
  return m_prev;
}

ThreadMessage* ThreadMessage::getNext()
{
  return m_next;
}

ThreadMessage* ThreadMessage::getLastOnStack()
{
  return m_lastOnStack;
}

ThreadMessage& ThreadMessage::getNextOnStack()
{
  return *reinterpret_cast<ThreadMessage*>(reinterpret_cast<char*>(this) + m_size);
}

ThreadMessage::State ThreadMessage::getState() const
{
  return m_state;
}

uint32_t ThreadMessage::getSize() const
{
  return m_size;
}

void* ThreadMessage::getDataArea()
{
  return &m_data;
}

ThreadMessage* ThreadMessage::getMessage(void* dataArea)
{
  return reinterpret_cast<ThreadMessage*>(reinterpret_cast<char*>(dataArea) - sizeof(ThreadMessage) + sizeof(uint32_t));
}

void ThreadMessage::setSize(uint32_t size)
{
  m_size = size;
}

void ThreadMessage::setPrev(ThreadMessage* prev)
{
  m_prev = prev;
}

void ThreadMessage::setNext(ThreadMessage* next)
{
  m_next = next;
}

void ThreadMessage::setLastOnStack(ThreadMessage* lastOnStack)
{
  m_lastOnStack = lastOnStack;
}

void ThreadMessage::setState(State state)
{
  m_state = state;
}

uint32_t& ThreadMessage::getSecondMagic()
{
  return *reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(&m_data) + m_size - sizeof(ThreadMessage));
}
