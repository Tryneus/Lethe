#ifndef _LINUXHANDLETRANSFER_H
#define _LINUXHANDLETRANSFER_H

#include "Lethe.h"
#include <string>
#include <sys/socket.h>
#include <sys/un.h>

namespace lethe
{
  class LinuxHandleTransfer
  {
  public:
    LinuxHandleTransfer(uint32_t remoteProcessId, // Process to connect with
                        uint32_t timeout);  // Amount of time to allow

    LinuxHandleTransfer(ByteStream& stream, // Stream to synchronize with
                        uint32_t timeout);  // Amount of time to allow

    ~LinuxHandleTransfer();

    void sendPipe(const Pipe& pipe);
    void sendTimer(const Timer& timer);
    void sendEvent(const Event& event);
    void sendMutex(const Mutex& mutex);
    void sendSemaphore(const Semaphore& semaphore);

    Pipe* recvPipe(uint32_t timeout);
    Timer* recvTimer(uint32_t timeout);
    Event* recvEvent(uint32_t timeout);
    Mutex* recvMutex(uint32_t timeout);
    Semaphore* recvSemaphore(uint32_t timeout); // warning - semaphore max cannot be enforced cross-process

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxHandleTransfer(const LinuxHandleTransfer&);
    LinuxHandleTransfer& operator = (const LinuxHandleTransfer&);

    static const std::string s_udsPath;
    static const std::string s_udsBaseName;
    static Atomic32 s_uniqueId;

    std::string m_name;
    Handle m_socket;
    uint64_t m_endTime;

    static const char s_semaphoreType = 'S';
    static const char s_timerType = 'T';
    static const char s_eventType = 'E';
    static const char s_mutexType = 'M';
    static const char s_pipeType = 'P';

    bool determineServer(ByteStream& stream);
    void initializeServer(ByteStream& stream, sockaddr_un& addr, socklen_t addrLength);
    void initializeClient(ByteStream& stream, sockaddr_un& addr, socklen_t addrLength);

    void sendInternal(Handle handle, char handleType);
    Handle recvInternal(char handleType, uint32_t timeout);
  };
}

#endif
