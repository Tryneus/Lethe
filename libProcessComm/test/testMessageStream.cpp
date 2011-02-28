#include "Lethe.h"
#include "ProcessComm.h"
#include "ThreadComm.h"
#include "catch.hpp"

using namespace lethe;

class MessageStreamThread : public Thread
{
private:
  ByteStream& m_byteStream;
  MessageStream* m_messageStream;

public:
  MessageStreamThread(ByteStream& byteStream) :
    Thread(INFINITE),
    m_byteStream(byteStream),
    m_messageStream(NULL)
  {
    // Do nothing
  }

  ~MessageStreamThread()
  {
    delete m_messageStream;
  }

  void setup()
  {
    m_messageStream = new ProcessMessageStream(m_byteStream, 2000, 1000);
  }

  void iterate(Handle handle GCC_UNUSED)
  {
    // Do nothing
  }
};

TEST_CASE("messageStream/structor", "Test construction and destruction of a ProcessMessageStream")
{
  // Use ThreadByteStreams since we're in the same process
  for(uint32_t i = 0; i < 1000; ++i)
  {
    ThreadByteConnection byteConnection;
    ByteStream& byteStream = byteConnection.getStreamA();
    MessageStreamThread thread(byteConnection.getStreamB());

    thread.start();

    ProcessMessageStream messageStream(byteStream, 2000, 1000);

    thread.stop();
    REQUIRE(WaitForObject(thread, 1000) == WaitSuccess);
    REQUIRE(thread.getError() == "");
  }
}
