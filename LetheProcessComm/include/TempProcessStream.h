#ifndef _TEMPPROCESSSTREAM_H
#define _TEMPPROCESSSTREAM_H

#include "Lethe.h"
#include <string>

namespace lethe
{
  class TempProcessStream : public ByteStream
  {
  public:
    TempProcessStream(uint32_t remoteProcessId);
    ~TempProcessStream();

    bool flush(uint32_t timeout);
    void send(const void* buffer, uint32_t size);
    uint32_t receive(void* buffer, uint32_t size);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    TempProcessStream(const TempProcessStream&);
    TempProcessStream& operator = (const TempProcessStream&);

    static void acquireProcessLock(uint32_t processId);
    static void releaseProcessLock(uint32_t processId);

    static const std::string s_baseName;
    static std::set<uint32_t> s_processInfo;
    static Mutex s_mutex;

    ByteStream* m_stream;
    uint32_t m_remoteProcessId;
  };
}

#endif
