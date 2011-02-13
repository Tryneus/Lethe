#include "Channel.h"
#include "Message.h"
#include "AbstractionException.h"

using namespace ThreadComm;

Channel::Channel(Header& in,
                 Header& out) :
  WaitObject(INVALID_HANDLE_VALUE),
  m_in(in),
  m_out(out)
{
  setWaitHandle(m_in.getHandle());
}

Channel::~Channel()
{
  // Do nothing
}

void* Channel::allocate(uint32_t size)
{
  size += sizeof(Message);
  size += sizeof(uint64_t) - (size % sizeof(uint64_t)); // Align along 64-bit boundary

  if(size < sizeof(Message))
    throw std::bad_alloc();

  return m_out.allocate(size).getDataArea();
}

void Channel::send(void* msg)
{
  Message* message = Message::getMessage(msg);

  if(msg == NULL) return;

  if(!message->overflowCheck())
    throw std::runtime_error("buffer overflow");

  if(message->getState() != Message::Alloc)
    throw std::invalid_argument("buffer in the wrong state");

  m_out.send(*message);
}

void* Channel::receive()
{
  return m_in.receive().getDataArea();
}

void  Channel::release(void* msg)
{
  Message* message = Message::getMessage(msg);

  if(msg == NULL) return;

  if(!message->overflowCheck())
    throw std::runtime_error("buffer overflow");

  if(!m_in.release(*message) && !m_out.release(*message))
    throw std::invalid_argument("invalid buffer");
}
