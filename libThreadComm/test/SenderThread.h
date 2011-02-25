#include "Lethe.h"

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
