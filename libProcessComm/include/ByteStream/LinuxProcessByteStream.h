#ifndef _LINUXPROCESSBYTESTREAM_H
#define _LINUXPROCESSBYTESTREAM_H

#include "Lethe.h"
#include <cstdatomic>

// Prototype of the hash map, so users don't need the include
namespace mct
{
  template<typename, typename, typename, typename, typename, bool>
  class closed_hash_map;
}

namespace lethe
{
  class LinuxProcessByteStream : public ByteStream
  {
  public:
    // Use fifos to negotiate anonymous bytestream setup - named by pair: <pid>-to-<pid>
    LinuxProcessByteStream(uint32_t processId, uint32_t timeout);
    // If there is already a bytestream, we don't have to go through so much trouble to create another one
    LinuxProcessByteStream(ByteStream& stream, uint32_t timeout);
    ~LinuxProcessByteStream();

    operator WaitObject&();
    Handle getHandle() const;

    void send(const void* buffer, uint32_t size);
    uint32_t receive(void* buffer, uint32_t size);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxProcessByteStream(const LinuxProcessByteStream&);
    LinuxProcessByteStream& operator = (const LinuxProcessByteStream&);

    static Mutex* getProcessMutex(uint32_t processId);
    static void removeProcessMutex(uint32_t processId);
    static Mutex s_mutex;
    static mct::closed_hash_map<uint32_t,
                                Mutex*,
                                std::tr1::hash<uint32_t>,
                                std::equal_to<uint32_t>,
                                std::allocator<std::pair<const uint32_t, Mutex*> >,
                                false>* s_processInfo;

    void doSetup(ByteStream& stream, uint32_t endTime);

    Pipe* m_pipeIn;
    Pipe* m_pipeOut;
  };
}

#endif
