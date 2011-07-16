#ifndef _LINUXHANDLETRANSFER_H
#define _LINUXHANDLETRANSFER_H

#include "Lethe.h"
#include <string>

namespace lethe
{
  class LinuxHandleTransfer
  {
  public:
    LinuxHandleTransfer(ByteStream& stream, // Stream to synchronize with
                        uint32_t timeout);  // Amount of time to allow

    ~LinuxHandleTransfer();

    // Mutex excluded - locking thread enforcement cannot be guaranteed
    void sendPipe(const Pipe& pipe);
    void sendTimer(const Timer& timer);
    void sendEvent(const Event& event);
    void sendSemaphore(const Semaphore& semaphore);

    Pipe* recvPipe(uint32_t timeout);
    Timer* recvTimer(uint32_t timeout);
    Event* recvEvent(uint32_t timeout);
    Semaphore* recvSemaphore(uint32_t timeout); // warning - semaphore max cannot be enforced cross-process

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxHandleTransfer(const LinuxHandleTransfer&);
    LinuxHandleTransfer& operator = (const LinuxHandleTransfer&);

    static void synchronize(ByteStream& stream, std::string& name, bool& isServer, uint32_t timeout);
    static const std::string s_udsPath;
    static const std::string s_udsBaseName;
    std::string m_name;
    Handle m_socket;

    static const char s_semaphoreType = 'S';
    static const char s_timerType = 'T';
    static const char s_eventType = 'E';
    static const char s_pipeType = 'P';

    void sendInternal(Handle handle, char handleType);
    Handle recvInternal(char handleType, uint32_t timeout);
  };
}

#endif