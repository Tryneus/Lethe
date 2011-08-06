#ifndef _COMMREGISTRY_H
#define _COMMREGISTRY_H

#include "Lethe.h"
#include "ThreadComm.h"
#include "ProcessComm.h"
#include "SocketComm.h"
#include "StaticSingleton.h"

// Prototype of the hash map, so users don't need the include
namespace mct
{
  template<typename, typename, typename, typename, typename, bool>
  class closed_hash_map;
}

namespace lethe
{
  class CommRegistry : StaticSingleton<CommRegistry>
  {
  public:
    std::pair<ByteStream*, ByteStream*> createThreadByteStream();
    ByteStream* createProcessByteStream(uint32_t processId, uint32_t timeout);
    ByteStream* createSocketByteStream(const std::string& hostname, uint32_t timeout);

    std::pair<MessageStream*, MessageStream*> createThreadMessageStream();
    MessageStream* createProcessMessageStream(uint32_t processId, uint32_t timeout);
    MessageStream* createSocketMessageStream(const std::string& hostname, uint32_t timeout);

    void destroyStream(ByteStream& stream);
    void destroyStream(MessageStream& stream);

    enum StreamType
    {
      ThreadByte,
      ThreadMessage,
      ProcessByte,
      ProcessMessage,
      SocketByte,
      SocketMessage
    };

    // Callback functions are passed a pointer to the new stream and an enum of the type of stream it is
    // They return a bool, true = everything is fine, false = destroy the stream
    typedef bool(*CallbackFunction)(void*, StreamType);

    // Incoming connection behavior
    // In synchronous mode, the user must call accept() to listen for new connections
    // In asynchronous mode, the CommRegistry uses an internal thread to listen for new connections
    // The callback function will be called any time a new incoming connection is received
    // The callback thread will have new connections added as wait objects
    void setMode(bool asynchronous, CallbackFunction* callbackFunction, Thread* callbackThread);

    void setMessageStreamSize(uint32_t size);
    uint32_t getMessageStreamSize() const;

    void addSocketByteStreamListener(const std::string& host, uint16_t port);
    void addSocketMessageStreamListener(const std::string& host, uint16_t port);

    // synchronous accept functions
    void* accept(StreamType& type, uint32_t timeout); // Accepts a single stream or blocks until timeout expires, the new stream is returned
    uint32_t accept(uint32_t count, uint32_t timeout); // Accepts up to count streams, or until timeout expires, will fail if no callback is set

  private:
    friend class StaticSingleton<CommRegistry>;

    CommRegistry();
    ~CommRegistry();

    static const std::string s_pipeNameBase;
    static std::string getPipeName(uint32_t processId);

    void destroyInternal(Handle handle);
    void destroyThreadByteConnection(void* conn);
    void destroyThreadMessageConnection(void* conn);
    void* acceptWait(StreamType& type, uint32_t timeout, uint32_t endTime);
    void* receiveProcessStream(StreamType& type, uint32_t timeout);

    Mutex m_mutex;

    struct ConnectionInfo
    {
      ConnectionInfo(StreamType type, Handle handle, void* stream, void* connection);

      StreamType m_type;
      Handle m_handle;
      void* m_stream;
      void* m_connection;
    };

    struct StreamInfo
    {
      StreamInfo(StreamType type, uint32_t processId);
      StreamInfo(ByteStream& pipe);

      StreamType m_type;
      uint32_t m_processId;
    };

    mct::closed_hash_map<Handle,
                         ConnectionInfo*,
                         std::tr1::hash<Handle>,
                         std::equal_to<Handle>,
                         std::allocator<std::pair<const Handle, ConnectionInfo*> >,
                         false>* m_streams;

    class CommThread : public Thread
    {
    public:
      CommThread(CommRegistry& parent);
      ~CommThread();

    private:
      void iterate(Handle handle);

      CommRegistry& m_parent;
    }* m_internalThread; // If in synchronous mode, this will be NULL

    friend class CommThread; // Give CommThread access to members

    Thread* m_callbackThread;
    CallbackFunction* m_callbackFunction;

    // Named pipe to receive inter-process connection requests
    Pipe m_pipeIn;
    WaitSet m_waitSet;

    mct::closed_hash_map<Handle,
                         SocketByteStreamListener*,
                         std::tr1::hash<Handle>,
                         std::equal_to<Handle>,
                         std::allocator<std::pair<const Handle, SocketByteStreamListener*> >,
                         false>* m_byteListeners;

    mct::closed_hash_map<Handle,
                         SocketMessageStreamListener*,
                         std::tr1::hash<Handle>,
                         std::equal_to<Handle>,
                         std::allocator<std::pair<const Handle, SocketMessageStreamListener*> >,
                         false>* m_messageListeners;

    static const uint32_t s_defaultSocketListenerQueueLength;
    uint32_t m_defaultSocketListenerQueueLength;
    static const uint32_t s_defaultMessageStreamSize;
    uint32_t m_defaultMessageStreamSize;

    static const uint32_t s_mutexTimeout;
  };
}

#endif
