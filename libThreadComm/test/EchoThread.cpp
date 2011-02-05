#include "EchoThread.h"
#include "Exception.h"
#include "Log.h"

EchoThread::EchoThread(ThreadComm::Channel& channel) :
  m_channel(channel),
  m_chanHandle(m_channel.getHandle()),
  m_iterationCount(0),
  m_repliesToSend(0)
{
  LogInfo("Echo thread handle: " << (uint32_t)getHandle());
  addWaitObject(m_channel);
  setWaitTimeout(INFINITE);
}

EchoThread::~EchoThread()
{
  // Try to push through any messages left on the line, if we run out of space, oh well
  try
  {
    while(true)
      receiveMessage();
  }
  catch(Exception& ex)
  {
    if(ex.what() != "Receive called with nothing to receive")
      throw;
  }

  sendReplies();

  LogInfo("Echo thread performed " << m_iterationCount << " iterations");
}

void EchoThread::iterate(Handle handle)
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
  if(msg[0] != 1) throw Exception("Incorrect message data");
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
  catch(OutOfMemoryException&) { }
}

void EchoThread::abandoned(Handle handle)
{
  if(handle == m_channel.getHandle())
    throw Exception("Problem with the channel's handle");
  else
    LogError("Unrecognized handle abandoned");
}
