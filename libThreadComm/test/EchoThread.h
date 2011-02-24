#include "Abstraction.h"

class EchoThread : public Thread
{
public:
  EchoThread(MessageStream& channel);
  ~EchoThread();

private:
  void iterate(Handle handle);
  void abandoned(Handle handle);
  void receiveMessage();
  void sendReplies();

  MessageStream& m_channel;
  uint32_t m_iterationCount;
  uint32_t m_repliesToSend;
};
