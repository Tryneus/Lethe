#include "Channel.h"
#include "Message.h"
#include "Exception.h"

using namespace ThreadComm;

Channel::Channel(Header& in,
                 Header& out) :
  m_in(in),
  m_out(out)
{
  // Do nothing
}

Channel::~Channel()
{
  // Do nothing
}

Handle Channel::getHandle()
{
  return m_in.getHandle();
}

void* Channel::allocate(uint32_t size)
{
  size += sizeof(Message);
  size += sizeof(uint64_t) - (size % sizeof(uint64_t));

  if(size < sizeof(Message))
    throw Exception("Size too large");

  Message* message = m_out.allocate(size);

  if(message == NULL)
    throw Exception("Out of memory");

  return message->getDataArea();
}

void Channel::send(void* msg)
{
  Message* message = Message::getMessage(msg);

  if(msg == NULL) return;

  if(!message->overflowCheck())
    throw Exception("Buffer overflow detected in message being sent");

  if(message->getState() != Message::Alloc)
    throw Exception("Attempt to send a message in the wrong state");

  m_out.send(message);
}

void* Channel::receive()
{
  Message* message = m_in.receive();

  if(message != NULL)
    return message->getDataArea();

  return NULL;
}

void  Channel::release(void* msg)
{
  Message* message = Message::getMessage(msg);

  if(msg == NULL) return;

  if(!message->overflowCheck())
    throw Exception("Buffer overflow detected in message being released");

  if(!m_in.release(message) && !m_out.release(message))
    throw Exception("Released buffer does not belong to this channel");
}
