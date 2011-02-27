#include "MessageStream/ProcessMessage.h"
#include "MessageStream/ProcessMessageHeader.h"
#include "Lethe.h"
#include "Log.h"

#define FIRST_MAGIC 0x849D07F3 // Guaranteed to be random, chosen by a fair dice roll
#define SECOND_MAGIC ~FIRST_MAGIC

using namespace lethe;

ProcessMessage::ProcessMessage(uint32_t offset, uint32_t size, State state) :
  m_offset(offset),
  m_prev(0),
  m_next(0),
  m_lastOnStack(0),
  m_size(size),
  m_state(state),
  m_magic(FIRST_MAGIC)
{
  getSecondMagic() = SECOND_MAGIC;
}

uint32_t ProcessMessage::getEnd()
{
  return reinterpret_cast<ProcessMessageHeader*>(reinterpret_cast<uint8_t*>(this) - m_offset)->getSize();
}

ProcessMessage* ProcessMessage::split(uint32_t size)
{
  uint32_t extraSize = m_size - size;
  ProcessMessage* extra = NULL;

  if(extraSize > sizeof(ProcessMessage))
  {
    m_size = size;

    extra = new (getMessage(getNextOnStack()))
      ProcessMessage(getNextOnStack(), extraSize, ProcessMessage::Free);

    extra->setLastOnStack(m_offset);
    getSecondMagic() = SECOND_MAGIC;

    if(extra->getNextOnStack() < getEnd())
    {
      getMessage(extra->getNextOnStack())->setLastOnStack(extra->getOffset());
    }
  }

  return extra;
}

bool ProcessMessage::overflowCheck()
{
  return (m_magic == FIRST_MAGIC) && (getSecondMagic() == SECOND_MAGIC);
}

uint32_t ProcessMessage::getOffset()
{
  return m_offset;
}

uint32_t ProcessMessage::getPrev()
{
  return m_prev;
}

uint32_t ProcessMessage::getNext()
{
  return m_next;
}

uint32_t ProcessMessage::getLastOnStack()
{
  return m_lastOnStack;
}

uint32_t ProcessMessage::getNextOnStack()
{
  return m_offset + m_size;
}

ProcessMessage::State ProcessMessage::getState() const
{
  return m_state;
}

uint32_t ProcessMessage::getSize() const
{
  return m_size;
}

void* ProcessMessage::getDataArea()
{
  return &m_data;
}

ProcessMessage* ProcessMessage::getMessage(uint32_t offset)
{
  return reinterpret_cast<ProcessMessage*>(reinterpret_cast<uint8_t*>(this) - m_offset + offset);
}

ProcessMessage* ProcessMessage::getMessage(void* dataArea)
{
  return reinterpret_cast<ProcessMessage*>(reinterpret_cast<uint8_t*>(dataArea) - sizeof(ProcessMessage) + sizeof(uint32_t));
}

void ProcessMessage::setSize(uint32_t size)
{
  m_size = size;
}

void ProcessMessage::setPrev(uint32_t prev)
{
  m_prev = prev;
}

void ProcessMessage::setNext(uint32_t next)
{
  m_next = next;
}

void ProcessMessage::setLastOnStack(uint32_t lastOnStack)
{
  m_lastOnStack = lastOnStack;
}

void ProcessMessage::setState(State state)
{
  m_state = state;
}

uint32_t& ProcessMessage::getSecondMagic()
{
  return *reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(&m_data) + m_size - sizeof(ProcessMessage));
}
