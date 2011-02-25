#include "EchoThread.h"
#include "LetheException.h"
#include "Log.h"

EchoThread::EchoThread(lethe::MessageStream& channel) :
  lethe::Thread(INFINITE),
  m_channel(channel),
  m_iterationCount(0),
  m_repliesToSend(0)
{
  LogInfo("Echo thread handle: " << getHandle());
  addWaitObject(m_channel);
}

EchoThread::~EchoThread()
{
  // Try to get any messages left on the line
  try
  {
    while(true)
      receiveMessage();
  }
  catch(std::logic_error&)
  {
    // A logic error is thrown when receive is called with nothing to receive
  }

  // If this runs out of space, oh well
  sendReplies();

  LogInfo("Echo thread performed " << m_iterationCount << " iterations");
}

void EchoThread::iterate(lethe::Handle handle)
{
  if(handle == m_channel.getHandle())
    receiveMessage();
  if(handle == INVALID_HANDLE_VALUE)
    LogInfo("Timeout");

  sendReplies();
  ++m_iterationCount;
}

void EchoThread::receiveMessage()
{
  // Receive a message
  uint32_t* msg = reinterpret_cast<uint32_t*>(m_channel.receive());
  if(msg[0] != 1) throw std::logic_error("invalid data");
  m_channel.release(msg);
  ++m_repliesToSend;
}

void EchoThread::sendReplies()
{
  uint32_t* msg = NULL;

  try
  {
    while(m_repliesToSend > 0)
    {
      msg = reinterpret_cast<uint32_t*>(m_channel.allocate(sizeof(uint32_t)));
      msg[0] = 2;
      m_channel.send(msg);
      --m_repliesToSend;
    }
  }
  catch(std::bad_alloc&) { }
}

void EchoThread::abandoned(lethe::Handle handle)
{
  if(handle == m_channel.getHandle())
    throw std::logic_error("handle abandoned");
  else
    LogError("Unrecognized handle abandoned");
}
