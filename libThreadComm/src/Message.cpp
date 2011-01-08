#include "Message.h"
#include "Header.h"
#include "Abstraction.h"

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

    extra = new (getNextOnStack())
      Message(m_header, extraSize, Message::Free);

    extra->m_lastOnStack = this;
    getSecondMagic() = SECOND_MAGIC;

    if(extra->getNextOnStack() < m_header->getEndPtr())
    {
      extra->getNextOnStack()->setLastOnStack(extra);
    }
  }

  return extra;
}

bool Message::overflowCheck()
{
  return (m_magic == FIRST_MAGIC) && (getSecondMagic() == SECOND_MAGIC);
}
