#include "SenderThread.h"
#include "AbstractionException.h"
#include "Log.h"

SenderThread::SenderThread(ThreadComm::Channel& channel) :
  Thread(200),
  m_channel(channel),
  m_iterationCount(0),
  m_messagesSent(0),
  m_messagesReleased(0)
{
  LogInfo("Sender thread handle: " << (uint32_t)getHandle());
  addWaitObject(m_channel);
}

SenderThread::~SenderThread()
{
  // Receive any messages left on the line before exiting
  try
  {
    while(true)
      receiveMessage();
  }
  catch(std::logic_error&)
  {
    // A logic error is thrown when receive is called with nothing to receive
  }

  LogInfo("Sender thread performed " << m_iterationCount << " iterations");
  LogInfo("Sender sent " << m_messagesSent << ", received " << m_messagesReleased);

  if(m_messagesSent != m_messagesReleased)
  {
    LogError("Did not receive all replies");
  }
}

void SenderThread::iterate(Handle handle)
{
  if(handle == m_channel.getHandle())
    receiveMessage();

  if(m_messagesSent == m_messagesReleased)
    sendMessages();
  else if(handle == INVALID_HANDLE_VALUE)
    LogInfo("Sender timed out waiting for all responses");

  ++m_iterationCount;
}

void SenderThread::sendMessages()
{
  // Send a random number of messages, 1 - 400
  uint32_t messagesToSend = rand() % 400 + 1;
  uint32_t i = 0;
  uint32_t* msg = NULL;

  try
  {
    while(i++ < messagesToSend)
    {
      msg = reinterpret_cast<uint32_t*>(m_channel.allocate(sizeof(uint32_t)));
      msg[0] = 1;
      m_channel.send(msg);
      ++m_messagesSent;
    }
  }
  catch(std::bad_alloc&) { }
}

void SenderThread::abandoned(Handle handle)
{
  if(handle == m_channel.getHandle())
    throw std::runtime_error("handle abandoned");
  else
    LogError("Unrecognized handle abandoned");
}

void SenderThread::receiveMessage()
{
  // Receive a message
  uint32_t* msg = reinterpret_cast<uint32_t*>(m_channel.receive());
  if(msg[0] != 2)
    throw std::logic_error("invalid data");
  m_channel.release(msg);
  ++m_messagesReleased;
}
