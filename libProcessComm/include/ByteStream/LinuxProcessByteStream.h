#ifndef _LINUXPROCESSBYTESTREAM_H
#define _LINUXPROCESSBYTESTREAM_H

#include "Lethe.h"

namespace lethe
{
  class LinuxProcessByteStream : public ByteStream
  {
  public:
    LinuxProcessByteStream(const std::string& pipeIn, const std::string& pipeOut, uint32_t timeout);
    ~LinuxProcessByteStream();

    operator WaitObject&();
    Handle getHandle() const;

    void send(void* buffer, uint32_t size);
    uint32_t receive(void* buffer, uint32_t size);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxProcessByteStream(const LinuxProcessByteStream&);
    LinuxProcessByteStream& operator = (const LinuxProcessByteStream&);

    static const uint32_t s_maxAsyncEvents = 10;

    void asyncWrite(const void* buffer, uint32_t bufferSize);
    void getAsyncResults();

    Handle m_pipeIn;
    Handle m_pipeOut;
    uint32_t m_asyncStart;
    uint32_t m_asyncEnd;
    struct aiocb m_asyncArray[s_maxAsyncEvents];
    bool m_blockingWrite;
  };
}

#endif
