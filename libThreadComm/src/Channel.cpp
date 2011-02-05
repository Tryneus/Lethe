#include "Channel.h"
#include "Message.h"
#include "Exception.h"

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
    throw Exception("Size too large");

  return m_out.allocate(size).getDataArea();
}

void Channel::send(void* msg)
{
  Message* message = Message::getMessage(msg);

  if(msg == NULL) return;

  if(!message->overflowCheck())
    throw Exception("Buffer overflow detected in message being sent");

  if(message->getState() != Message::Alloc)
    throw Exception("Attempt to send a message in the wrong state");

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
    throw Exception("Buffer overflow detected in message being releasedi, or invalid message buffer");

  if(!m_in.release(*message) && !m_out.release(*message))
    throw Exception("Released buffer does not belong to this channel");
}
