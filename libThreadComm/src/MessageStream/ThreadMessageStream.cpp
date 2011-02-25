#include "MessageStream/ThreadMessageStream.h"
#include "MessageStream/ThreadMessage.h"
#include "LetheException.h"

using namespace lethe;

ThreadMessageStream::ThreadMessageStream(ThreadMessageHeader& in,
                                         ThreadMessageHeader& out,
                                         WaitObject& obj) :
  m_in(in),
  m_out(out),
  m_waitObject(obj)
{
  // Do nothing
}

ThreadMessageStream::~ThreadMessageStream()
{
  // Do nothing
}

ThreadMessageStream::operator WaitObject&()
{
  return m_waitObject;
}

Handle ThreadMessageStream::getHandle() const
{
  return m_waitObject.getHandle();
}

void* ThreadMessageStream::allocate(uint32_t size)
{
  size += sizeof(ThreadMessage);
  size += sizeof(uint64_t) - (size % sizeof(uint64_t)); // Align along 64-bit boundary

  if(size < sizeof(ThreadMessage))
    throw std::bad_alloc();

  return m_out.allocate(size).getDataArea();
}

void ThreadMessageStream::send(void* msg)
{
  ThreadMessage* message = ThreadMessage::getMessage(msg);

  if(msg == NULL) return;

  if(!message->overflowCheck())
    throw std::runtime_error("buffer overflow");

  if(message->getState() != ThreadMessage::Alloc)
    throw std::invalid_argument("buffer in the wrong state");

  m_out.send(*message);
}

void* ThreadMessageStream::receive()
{
  return m_in.receive().getDataArea();
}

void ThreadMessageStream::release(void* msg)
{
  ThreadMessage* message = ThreadMessage::getMessage(msg);

  if(msg == NULL) return;

  if(!message->overflowCheck())
    throw std::runtime_error("buffer overflow");

  if(!m_in.release(*message) && !m_out.release(*message))
    throw std::invalid_argument("invalid buffer");
}
