#include "Lethe.h"
#include "LetheException.h"
#include "ThreadComm.h"
#include "Log.h"
#include "catch/catch.hpp"

class SenderThread : public lethe::Thread
{
public:
  SenderThread(lethe::MessageStream& channel);
  ~SenderThread();

private:
  void iterate(lethe::Handle handle);
  void abandoned(lethe::Handle handle);
  void receiveMessage();
  void sendMessages();

  lethe::MessageStream& m_channel;
  uint32_t m_iterationCount;
  uint32_t m_messagesSent;
  uint32_t m_messagesReleased;
};

SenderThread::SenderThread(lethe::MessageStream& channel) :
  lethe::Thread(100),
  m_channel(channel),
  m_iterationCount(0),
  m_messagesSent(0),
  m_messagesReleased(0)
{
  LogInfo("Sender thread handle: " << getHandle());
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

void SenderThread::iterate(lethe::Handle handle)
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

void SenderThread::abandoned(lethe::Handle handle)
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

class EchoThread : public lethe::Thread
{
public:
  EchoThread(lethe::MessageStream& channel);
  ~EchoThread();

private:
  void iterate(lethe::Handle handle);
  void abandoned(lethe::Handle handle);
  void receiveMessage();
  void sendReplies();

  lethe::MessageStream& m_channel;
  uint32_t m_iterationCount;
  uint32_t m_repliesToSend;
};

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

TEST_CASE("messageStream/stress", "Test echoing messages between threads at high rates")
{
  uint32_t testSeconds = 10;
  SenderThread* sender(NULL);
  EchoThread* echo(NULL);
  lethe::ThreadMessageConnection* conn(NULL);

  // Seed RNG
  {
    uint32_t randomSeed = lethe::seedRandom(0);
    LogInfo("Test seeded with " << randomSeed);
  }

  try
  {
    conn = new lethe::ThreadMessageConnection(500000, 500000);

    // Dynamically allocated so we can destroy them in order
    sender = new SenderThread(conn->getStreamA());
    echo = new EchoThread(conn->getStreamB());

    echo->start();
    sender->start();

    // Let the threads run for some seconds, then stop them
    lethe::sleep_ms(testSeconds * 1000);

    sender->stop();
    echo->stop();

    // Wait for threads to exit
    lethe::WaitSet waitSet;
    lethe::Handle finished;

    waitSet.add(*sender);
    waitSet.add(*echo);

    LogInfo("Waiting for threads to exit");
    while(waitSet.getSize() != 0)
    {
      if(waitSet.waitAny(2000, finished) != lethe::WaitSuccess)
        throw std::runtime_error("threads did not stop correctly");

      waitSet.remove(finished);
      LogInfo("Thread " << finished << " exited");
    }

    // Delete in order to push through messages on the line
    delete echo;
    delete sender;
    delete conn;
  }
  catch(std::exception& ex)
  {
    LogError("Exception encountered during test: " << ex.what());

    try
    {
      delete echo;
      delete sender;
      delete conn;
    }
    catch(std::exception& ex)
    {
      LogError("Another exception encountered while destroying threads: " << ex.what());
    }

    REQUIRE(false);
  }
}
