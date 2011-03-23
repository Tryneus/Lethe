#ifndef _WINDOWSHANDLETRANSFER_H
#define _WINDOWSHANDLETRANSFER_H

#include "Lethe.h"
#include <string>

namespace lethe
{
  class WindowsHandleTransfer
  {
  public:
    WindowsHandleTransfer(ByteStream& stream, // Stream to synchronize with
                          uint32_t timeout);  // Amount of time to allow

    ~WindowsHandleTransfer();

    // Mutex excluded due to limitations in Linux
    void sendPipe(const Pipe& pipe);
    void sendTimer(const Timer& timer);
    void sendEvent(const Event& event);
    void sendSemaphore(const Semaphore& semaphore);

    Pipe* recvPipe(uint32_t timeout);
    Timer* recvTimer(uint32_t timeout);
    Event* recvEvent(uint32_t timeout);
    Semaphore* recvSemaphore(uint32_t timeout);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsHandleTransfer(const WindowsHandleTransfer&);
    WindowsHandleTransfer& operator = (const WindowsHandleTransfer&);

    static const char s_semaphoreType = 'S';
    static const char s_timerType = 'T';
    static const char s_eventType = 'E';
    static const char s_pipeType = 'P';

    void sendInternal(const std::string& name, char type);
    const std::string recvInternal(char type, uint32_t timeout);
  };
}

#endif
