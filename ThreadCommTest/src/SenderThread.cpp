#include "SenderThread.h"
#include "Exception.h"
#include "Log.h"

SenderThread::SenderThread(ThreadComm::Channel& channel) :
  m_channel(channel),
  m_chanHandle(m_channel.getHandle()),
  m_iterationCount(0),
  m_messagesSent(0),
  m_messagesReleased(0)
{
  LogInfo("Sender thread handle: " << (uint32_t)getHandle());
  addWaitObject(m_channel.getHandle());
  setWaitTimeout(200);
}

SenderThread::~SenderThread()
{
  LogInfo("Sender thread performed " << m_iterationCount << " iterations");
  LogInfo("Sender sent " << m_messagesSent << ", received " << m_messagesReleased);
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

  while(i++ < messagesToSend)
  {
    uint32_t* msg;
    try
    {
      msg = reinterpret_cast<uint32_t*>(m_channel.allocate(sizeof(uint32_t)));
      msg[0] = 1;
      m_channel.send(msg);
      ++m_messagesSent;
    }
    catch(Exception& ex)
    {
      if(ex.what() == "Semaphore full, other side needs to wait on it") Sleep(10);
      else if(isStopping()) break;
      else throw;
      LogInfo("Failed a send: " << m_messagesSent << " sent");
    }      
  }
}

void SenderThread::abandoned(Handle handle)
{
  if(handle == m_channel.getHandle())
    throw Exception("Problem with the channel's handle");
  else
    LogError("Unrecognized handle abandoned");
}

void SenderThread::receiveMessage()
{
  // Receive a message
  uint32_t* msg = reinterpret_cast<uint32_t*>(m_channel.receive());
  if(msg[0] != 2)
    throw Exception("Incorrect message data");
  m_channel.release(msg);
  ++m_messagesReleased;
}