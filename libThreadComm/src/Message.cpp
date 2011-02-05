#include "Message.h"
#include "Header.h"
#include "Abstraction.h"
#include "Log.h"

#define FIRST_MAGIC 0x849D07F3 // Guaranteed to be random, chosen by a fair dice roll
#define SECOND_MAGIC ~FIRST_MAGIC

using namespace ThreadComm;

Message::Message(Header* header, uint32_t size, State state) :
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

Message* Message::split(uint32_t size)
{
  uint32_t extraSize = m_size - size;
  Message* extra = NULL;

  if(extraSize > sizeof(Message))
  {
    m_size = size;

    extra = new (&getNextOnStack())
      Message(m_header, extraSize, Message::Free);

    extra->m_lastOnStack = this;
    getSecondMagic() = SECOND_MAGIC;

    if(&extra->getNextOnStack() < (void*)((uint8_t*)m_header->getEndPtr() - sizeof(Message)))
    {
      extra->getNextOnStack().setLastOnStack(extra);
    }
  }

  return extra;
}

bool Message::overflowCheck()
{
  return (m_magic == FIRST_MAGIC) && (getSecondMagic() == SECOND_MAGIC);
}

Header* Message::getHeader()
{
  return m_header;
}

Message* Message::getPrev()
{
  return m_prev;
}

Message* Message::getNext()
{
  return m_next;
}

Message* Message::getLastOnStack()
{
  return m_lastOnStack;
}

Message& Message::getNextOnStack()
{
  return *reinterpret_cast<Message*>(reinterpret_cast<char*>(this) + m_size);
}

Message::State Message::getState() const
{
  return m_state;
}

uint32_t Message::getSize() const
{
  return m_size;
}

void* Message::getDataArea()
{
  return &m_data;
}

Message* Message::getMessage(void* dataArea)
{
  return reinterpret_cast<Message*>(reinterpret_cast<char*>(dataArea) - sizeof(Message) + sizeof(uint32_t));
}

void Message::setSize(uint32_t size)
{
  m_size = size;
}

void Message::setPrev(Message* prev)
{
  m_prev = prev;
}

void Message::setNext(Message* next)
{
  m_next = next;
}

void Message::setLastOnStack(Message* lastOnStack)
{
  m_lastOnStack = lastOnStack;
}

void Message::setState(State state)
{
  m_state = state;
}

uint32_t& Message::getSecondMagic()
{
  return *reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(&m_data) + m_size - sizeof(Message));
}
