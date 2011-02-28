#ifndef _LINUXHANDLETRANSFER_H
#define _LINUXHANDLETRANSFER_H

#include "Lethe.h"
#include <string>

namespace lethe
{
  class LinuxHandleTransfer
  {
  public:
    LinuxHandleTransfer(ByteStream& stream,      // Stream to synchronize with
                        const std::string& name, // Name of Unix Domain Socket file
                        bool serverSide,         // True if this object should act as the server, false otherwise
                        uint32_t timeout);       // Amount of time to allow

    ~LinuxHandleTransfer();

    // Pipe excluded - use ProcessByteStream instead
    // Mutex excluded - locking thread enforcement cannot be guaranteed
    void sendTimer(const Timer& timer);
    void sendEvent(const Event& event);
    void sendSemaphore(const Semaphore& semaphore);

    Timer* recvTimer(uint32_t timeout);
    Event* recvEvent(uint32_t timeout);
    Semaphore* recvSemaphore(uint32_t timeout); // warning - semaphore max cannot be enforced cross-process

  private:
    static const std::string s_udsPath;
    static const std::string s_udsBaseName;
    std::string m_name;
    Handle m_socket;

    static const char s_anyType = 'A';
    static const char s_timerType = 'T';
    static const char s_eventType = 'E';
    static const char s_semaphoreType = 'S';

    void sendInternal(Handle handle, char handleType);
    Handle recvInternal(char handleType, uint32_t timeout);
  };
}

#endif
