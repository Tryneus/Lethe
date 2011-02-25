#ifndef _LINUXPROCESSMESSAGESTREAM_H
#define _LINUXPROCESSMESSAGESTREAM_H

#include "Lethe.h"

namespace lethe
{
  class ProcessMessageHeader;

  class LinuxProcessMessageStream
  {
  public:
    LinuxProcessMessageStream(uint32_t remoteProcessId, uint32_t outgoingSize);
    ~LinuxProcessMessageStream();

    operator WaitObject&();
    Handle getHandle() const;

    void* allocate(uint32_t size);
    void send(void* buffer);
    void* receive();
    void release(void* buffer);

  private:
    static const std::string getInName(uint32_t remoteProcessId);
    static const std::string getOutName(uint32_t remoteProcessId);

    const std::string m_inName;
    const std::string m_outName;

    // TODO: possible to share semaphores instead?
    Pipe m_pipeIn;
    Pipe m_pipeOut;

    SharedMemory* m_shmIn;
    SharedMemory* m_shmOut;

    ProcessMessageHeader* m_headerIn;
    ProcessMessageHeader* m_headerOut;
  };
}

#endif
