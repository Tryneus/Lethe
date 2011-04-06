#ifndef _LINUXPROCESSMESSAGESTREAM_H
#define _LINUXPROCESSMESSAGESTREAM_H

#include "Lethe.h"
#include "ProcessMessageHeader.h"
#include <cstdatomic>

namespace lethe
{
  class LinuxProcessMessageStream : public MessageStream
  {
  public:
    LinuxProcessMessageStream(ByteStream& stream, uint32_t outgoingSize, uint32_t timeout);
    LinuxProcessMessageStream(uint32_t processId, uint32_t outgoingSize, uint32_t timeout);
    ~LinuxProcessMessageStream();

    operator WaitObject&();
    Handle getHandle() const;

    void* allocate(uint32_t size);
    void send(void* buffer);
    void* receive();
    void release(void* buffer);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxProcessMessageStream(const LinuxProcessMessageStream&);
    LinuxProcessMessageStream& operator = (const LinuxProcessMessageStream&);

    static const std::string generateShmName();
    static const std::string s_nameBase;
    static std::atomic<uint32_t> s_nextId;

    static const std::string generateUdsName();
    static const std::string s_udsPath;

    static uint32_t checkSize(uint32_t size);

    static const uint32_t s_minSize = 20 * sizeof(ProcessMessage) + sizeof(ProcessMessageHeader);
    static const uint32_t s_maxSize = (1 << 25); // Limit: 32 MB

    void doSetup(ByteStream& stream, uint32_t timeout);
    void shutdown();

    Handle getLocalUds(const std::string& name);
    Handle getRemoteUds(const std::string& name);
    void sendHandle(Handle uds, Handle data);
    Handle recvHandle(Handle uds);

    Semaphore* m_semaphoreIn;
    Semaphore* m_semaphoreOut;

    SharedMemory* m_shmIn;
    SharedMemory* m_shmOut;

    ProcessMessageHeader* m_headerIn;
    ProcessMessageHeader* m_headerOut;
  };
}

#endif
