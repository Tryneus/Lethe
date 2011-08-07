#ifndef _BASEHANDLETRANSFER_H
#define _BASEHANDLETRANSFER_H

#include "Lethe.h"
#include <string>

// Prototype of the hash map, so users don't need the include
namespace mct
{
  template<typename, typename, typename, typename, typename, bool>
  class closed_hash_map;
}

namespace lethe
{
  class BaseHandleTransfer
  {
  public:
    BaseHandleTransfer(ByteStream& stream, // Stream to synchronize with
                       uint32_t timeout);  // Amount of time to allow

    BaseHandleTransfer(uint32_t remoteProcessId,
                       uint32_t timeout);  // Create own stream

    virtual ~BaseHandleTransfer();

    // Mutex excluded due to limitations in Linux
    virtual void sendPipe(const Pipe& pipe) = 0;
    virtual void sendTimer(const Timer& timer) = 0;
    virtual void sendEvent(const Event& event) = 0;
    virtual void sendSemaphore(const Semaphore& semaphore) = 0;

    virtual Pipe* recvPipe(uint32_t timeout) = 0;
    virtual Timer* recvTimer(uint32_t timeout) = 0;
    virtual Event* recvEvent(uint32_t timeout) = 0;
    virtual Semaphore* recvSemaphore(uint32_t timeout) = 0;

  protected:
    // Private, undefined copy constructor and assignment operator so they can't be used
    BaseHandleTransfer(const BaseHandleTransfer&);
    BaseHandleTransfer& operator = (const BaseHandleTransfer&);

    static const char s_semaphoreType = 'S';
    static const char s_timerType = 'T';
    static const char s_eventType = 'E';
    static const char s_pipeType = 'P';

    ByteStream* m_stream;

  private:
    static Mutex s_mutex;
    static void acquireProcessLock(uint32_t processId);
    static void releaseProcessLock(uint32_t processId);
    static mct::closed_hash_map<uint32_t,
                                Mutex*,
                                std::tr1::hash<uint32_t>,
                                std::equal_to<uint32_t>,
                                std::allocator<std::pair<const uint32_t, Mutex*> >,
                                false>* s_processInfo;
    static std::string s_syncString;

    bool m_createdStream;
  };
}

#endif
