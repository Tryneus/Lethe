#include "Lethe.h"

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
