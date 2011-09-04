#ifndef _PROCESSMESSAGESTREAM_H
#define _PROCESSMESSAGESTREAM_H

#include "Lethe.h"
#include "ProcessMessageHeader.h"
#include <cstdatomic>

namespace lethe
{
  class ProcessMessageStream : public MessageStream
  {
  public:
    ProcessMessageStream(ByteStream& stream, uint32_t outgoingSize, uint32_t timeout);
    ProcessMessageStream(uint32_t remoteProcessId, uint32_t outgoingSize, uint32_t timeout);
    ~ProcessMessageStream();

    operator WaitObject&();
    Handle getHandle() const;

    void* allocate(uint32_t size);
    void send(void* buffer);
    void* receive();
    void release(void* buffer);

    uint32_t size(void* buffer);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    ProcessMessageStream(const ProcessMessageStream&);
    ProcessMessageStream& operator = (const ProcessMessageStream&);

    static uint32_t checkSize(uint32_t size);

    static const std::string s_syncString;
    static const uint32_t s_minSize = 20 * sizeof(ProcessMessage) + sizeof(ProcessMessageHeader);
    static const uint32_t s_maxSize = (1 << 25); // Arbitrary limit: 32 MB

    void doSetup(ByteStream& stream, uint64_t endTime);
    void shutdown();

    // Objects used for outgoing messages, created locally, sent to the remote side
    SharedMemory m_shmOut;
    ProcessMessageHeader* m_headerOut;
    Semaphore m_semaphoreOut;

    // Objects used for incoming messages, created remotely, send to this side
    SharedMemory* m_shmIn;
    ProcessMessageHeader* m_headerIn;
    Semaphore* m_semaphoreIn;
  };
}

#endif
