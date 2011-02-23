#include "Abstraction.h"
#include "ThreadComm.h"

class EchoThread : public Thread
{
public:
  EchoThread(ThreadComm::Channel& channel);
  ~EchoThread();

private:
  void iterate(Handle handle);
  void abandoned(Handle handle);
  void receiveMessage();
  void sendReplies();

  ThreadComm::Channel& m_channel;
  uint32_t m_iterationCount;
  uint32_t m_repliesToSend;
};
