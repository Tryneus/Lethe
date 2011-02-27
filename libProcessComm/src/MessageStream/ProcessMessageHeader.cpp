#include "MessageStream/ProcessMessageHeader.h"
#include "LetheException.h"

using namespace lethe;

const uint32_t ProcessMessageHeader::s_firstBufferOffset = sizeof(ProcessMessageHeader);
const uint32_t ProcessMessageHeader::s_secondBufferOffset = sizeof(ProcessMessageHeader) + sizeof(ProcessMessage);
const uint32_t ProcessMessageHeader::s_thirdBufferOffset = sizeof(ProcessMessageHeader) + 2 * sizeof(ProcessMessage);

ProcessMessageHeader::ProcessMessageHeader(uint32_t size) :
  m_size(size),
  m_receiveList((uint8_t*)&m_receiveList - (uint8_t*)this, s_firstBufferOffset),
  m_releaseList((uint8_t*)&m_releaseList - (uint8_t*)this, s_secondBufferOffset),
  m_unallocList((uint8_t*)&m_unallocList - (uint8_t*)this, s_thirdBufferOffset, m_size)
{
  // Initialize buffers
  new ((uint8_t*)this + s_firstBufferOffset) ProcessMessage(s_firstBufferOffset, sizeof(ProcessMessage), ProcessMessage::Nil);

  ProcessMessage* secondMessage = new ((uint8_t*)this + s_secondBufferOffset)
    ProcessMessage(s_secondBufferOffset, sizeof(ProcessMessage), ProcessMessage::Pend);

  ProcessMessage* thirdMessage = new ((uint8_t*)this + s_thirdBufferOffset)
    ProcessMessage(s_thirdBufferOffset, m_size - 2 * sizeof(ProcessMessage), ProcessMessage::Free);

  secondMessage->setLastOnStack(s_firstBufferOffset);
  thirdMessage->setLastOnStack(s_secondBufferOffset);
}

ProcessMessageHeader::~ProcessMessageHeader()
{
  // TODO: write some indication of closure
}

uint32_t ProcessMessageHeader::getSize() const
{
  return m_size;
}

void* ProcessMessageHeader::allocate(uint32_t size)
{
  ProcessMessage* message = m_releaseList.pop();

  while(message != NULL)
  {
    m_unallocList.unallocate(message);
    message = m_releaseList.pop();
  }

  return m_unallocList.allocate(size);
}

void ProcessMessageHeader::send(void* buffer)
{
  ProcessMessage* message = reinterpret_cast<ProcessMessage*>(buffer);
  message->setState(ProcessMessage::Sent);
  m_receiveList.pushBack(message);
}

void* ProcessMessageHeader::receive()
{
  ProcessMessage* extraMessage;
  ProcessMessage* message = m_receiveList.receive(extraMessage);

  if(extraMessage != NULL)
    m_releaseList.pushBack(extraMessage);

  if(message == NULL)
    throw std::logic_error("nothing to receive");

  return message;
}

bool ProcessMessageHeader::release(void* buffer)
{
  ProcessMessage* message = reinterpret_cast<ProcessMessage*>(buffer);

  if(reinterpret_cast<void*>(message) < reinterpret_cast<void*>(this) ||
    reinterpret_cast<void*>(message) > reinterpret_cast<void*>((uint8_t*)this + m_size))
    return false; // Message didn't belong to this side, but it might belong to the other side

  switch(message->getState())
  {
  case ProcessMessage::Alloc:
    m_unallocList.unallocate(message);
    break;
  case ProcessMessage::Recv:
    message->setState(ProcessMessage::Pend);
    m_releaseList.pushBack(message);
    break;
  case ProcessMessage::Nil:
    message->setState(ProcessMessage::Pend);
    break;
  default:
    throw std::invalid_argument("buffer in the wrong state");
  }

  return true;
}
