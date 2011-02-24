#include "Abstraction.h"

class SenderThread : public Thread
{
public:
  SenderThread(MessageStream& channel);
  ~SenderThread();

private:
  void iterate(Handle handle);
  void abandoned(Handle handle);
  void receiveMessage();
  void sendMessages();

  MessageStream& m_channel;
  uint32_t m_iterationCount;
  uint32_t m_messagesSent;
  uint32_t m_messagesReleased;
};
