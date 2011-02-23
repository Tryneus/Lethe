#include "Abstraction.h"
#include "ThreadComm.h"

class SenderThread : public Thread
{
public:
  SenderThread(ThreadComm::Channel& channel);
  ~SenderThread();

private:
  void iterate(Handle handle);
  void abandoned(Handle handle);
  void receiveMessage();
  void sendMessages();

  ThreadComm::Channel& m_channel;
  uint32_t m_iterationCount;
  uint32_t m_messagesSent;
  uint32_t m_messagesReleased;
};
