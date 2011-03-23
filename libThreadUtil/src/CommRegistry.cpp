#include "CommRegistry.h"
#include "LetheInternal.h"
#include "mct/hash-map.hpp"
#include <sstream>

using namespace lethe;

const std::string CommRegistry::s_pipeNameBase = "lethe-commregistry-";
const uint32_t CommRegistry::s_defaultMessageStreamSize = 128 * 1024;

CommRegistry::CommRegistry() :
  m_streams(new mct::closed_hash_map<Handle, ConnectionInfo*>()),
  m_internalThread(NULL),
  m_callbackThread(NULL),
  m_callbackFunction(NULL),
  m_pipeIn(getPipeName(getProcessId()), true, "", false),
  m_defaultMessageStreamSize(s_defaultMessageStreamSize)
{
  m_waitSet.add(m_pipeIn);
}

CommRegistry::~CommRegistry()
{
  m_mutex.lock();

  // Clean up any remaining streams
  while(!m_streams->empty())
  {
    Handle handle = m_streams->begin()->first;
    destroyInternal(handle);
  }

  m_mutex.unlock(); // TODO: other threads may be waiting to perform an operation on a dead object
}

std::string CommRegistry::getPipeName(uint32_t processId)
{
  std::stringstream name;
  name << s_pipeNameBase << processId;
  return name.str();
}

void CommRegistry::setMessageStreamSize(uint32_t size)
{
  m_defaultMessageStreamSize = ((size == 0) ? s_defaultMessageStreamSize : size);
}

uint32_t CommRegistry::getMessageStreamSize() const
{
  return m_defaultMessageStreamSize;
}

CommRegistry::CommThread::CommThread(CommRegistry& parent) :
  Thread(INFINITE),
  m_parent(parent)
{
  // Do nothing
}

void CommRegistry::addSocketListener(uint32_t localIp, uint16_t port)
{
  std::stringstream host;

  // TODO: use system calls for this, so it supports ipv6 addresses
  host << ((localIp >> 24) & 0xFF) << "."
       << ((localIp >> 16) & 0xFF) << "."
       << ((localIp >> 8) & 0xFF)  << "."
       << (localIp & 0xFF);

  addSocketListener(host.str(), port);
}

void CommRegistry::addSocketListener(const std::string& host, uint16_t port)
{
  host.length();
  port = 0;
  // TODO: implement socket listeners, then this
  // SocketListener* listener = new SocketListener(host, port);
  // m_mutex.lock();
  // m_waitSet.add(*listener);
  // m_socketListeners.insert(listener);
  // m_mutex.unlock();
}

CommRegistry::CommThread::~CommThread()
{
  // Do nothing
}

void CommRegistry::CommThread::iterate(Handle handle)
{
  if(handle == m_parent.m_pipeIn.getHandle())
  {
    // Call CommRegistry::receiveConnection
  }
  else
  {
    // Socket stream, check which socket/port it's on
  }
}

ByteStream& CommRegistry::createByteStream(Thread& thread, uint32_t timeout GCC_UNUSED)
{
  ThreadByteConnection* conn = new ThreadByteConnection();

  ConnectionInfo* infoA = new ConnectionInfo(StreamType::ThreadByte, conn->getStreamA().getHandle(), &conn->getStreamA(), conn);
  ConnectionInfo* infoB = new ConnectionInfo(StreamType::ThreadByte, conn->getStreamB().getHandle(), &conn->getStreamB(), conn);
  m_streams->insert(std::make_pair(conn->getStreamA().getHandle(), infoA));
  m_streams->insert(std::make_pair(conn->getStreamB().getHandle(), infoB));

  thread.addWaitObject(conn->getStreamB());
  return conn->getStreamA();
}

ByteStream& CommRegistry::createByteStream(uint32_t processId, uint32_t timeout)
{
  // Open a one-way pipe for the remote process' CommRegistry and write a new stream indication
  Pipe tempPipe("", false, getPipeName(processId), false);
  StreamInfo sendInfo(StreamType::ProcessByte, getProcessId());

  tempPipe.send(&sendInfo, sizeof(sendInfo));

  // Now that the remote side has been notified, try to open the stream
  ProcessByteStream* stream = new ProcessByteStream(processId, timeout);
  ConnectionInfo* connInfo = new ConnectionInfo(StreamType::ProcessMessage, stream->getHandle(), stream, NULL);
  
  m_streams->insert(std::make_pair(stream->getHandle(), connInfo));

  return *stream;
}

ByteStream* CommRegistry::createByteStream(const std::string& hostname GCC_UNUSED, uint32_t timeout GCC_UNUSED)
{
  // TODO: implement
  return NULL;
}

MessageStream& CommRegistry::createMessageStream(Thread& thread, uint32_t timeout GCC_UNUSED)
{
  ThreadMessageConnection* conn = new ThreadMessageConnection(m_defaultMessageStreamSize, m_defaultMessageStreamSize);

  ConnectionInfo* infoA = new ConnectionInfo(StreamType::ThreadMessage, conn->getStreamA().getHandle(), &conn->getStreamA(), conn);
  ConnectionInfo* infoB = new ConnectionInfo(StreamType::ThreadMessage, conn->getStreamB().getHandle(), &conn->getStreamB(), conn);
  m_streams->insert(std::make_pair(conn->getStreamA().getHandle(), infoA));
  m_streams->insert(std::make_pair(conn->getStreamB().getHandle(), infoB));

  thread.addWaitObject(conn->getStreamB());
  return conn->getStreamA();
}

MessageStream& CommRegistry::createMessageStream(uint32_t processId, uint32_t timeout)
{
  // Open a one-way pipe for the remote process' CommRegistry and write a new stream indication
  Pipe tempPipe("", false, getPipeName(processId), false);
  StreamInfo sendInfo(StreamType::ProcessMessage, getProcessId());

  tempPipe.send(&sendInfo, sizeof(sendInfo));

  // Now that the remote side has been notified, try to open the stream
  ProcessMessageStream* stream = new ProcessMessageStream(processId, m_defaultMessageStreamSize, timeout);
  ConnectionInfo* connInfo = new ConnectionInfo(StreamType::ProcessMessage, stream->getHandle(), stream, NULL);
  
  m_streams->insert(std::make_pair(stream->getHandle(), connInfo));

  return *stream;
}

MessageStream* CommRegistry::createMessageStream(const std::string& hostname GCC_UNUSED, uint32_t timeout GCC_UNUSED)
{
  // TODO: implement
  return NULL;
}

void CommRegistry::destroyStream(ByteStream& stream)
{
  destroyInternal(stream.getHandle());
}

void CommRegistry::destroyStream(MessageStream& stream)
{
  destroyInternal(stream.getHandle());
}

void CommRegistry::destroyInternal(Handle handle)
{
  mct::closed_hash_map<Handle, ConnectionInfo*>::iterator i = m_streams->find(handle);
  
  if(i != m_streams->end())
  {
    switch(i->second->m_type)
    {
    case StreamType::ThreadByte:
      destroyThreadByteConnection(i->second->m_connection);
      break;

    case StreamType::ThreadMessage:
      destroyThreadMessageConnection(i->second->m_connection);
      break;

    case StreamType::ProcessByte:
    case StreamType::SocketByte:
      delete reinterpret_cast<ByteStream*>(i->second->m_stream);
      m_streams->erase(i);
      break;

    case StreamType::ProcessMessage:
    case StreamType::SocketMessage:
      delete reinterpret_cast<MessageStream*>(i->second->m_stream);
      m_streams->erase(i);
      break;

    default:
      throw std::logic_error("unexpected stream type in internal structure");
    }

  }
}

void CommRegistry::destroyThreadByteConnection(void* conn)
{
  ThreadByteConnection* connection = reinterpret_cast<ThreadByteConnection*>(conn);

  mct::closed_hash_map<Handle, ConnectionInfo*>::iterator a = m_streams->find(connection->getStreamA().getHandle());
  mct::closed_hash_map<Handle, ConnectionInfo*>::iterator b = m_streams->find(connection->getStreamB().getHandle());

  if(connection == NULL)
    throw std::logic_error("null connection in thread stream info");

  try
  {
    if(a == m_streams->end() || b == m_streams->end())
      throw std::logic_error("could not find both sides of thread stream");

    delete connection;
    m_streams->erase(a);
    m_streams->erase(b);
  }
  catch(...)
  {
    delete connection;

    if(a != m_streams->end())
      m_streams->erase(a);

    if(b != m_streams->end())
      m_streams->erase(b);

    throw;
  }
}

void CommRegistry::destroyThreadMessageConnection(void* conn)
{
  ThreadMessageConnection* connection = reinterpret_cast<ThreadMessageConnection*>(conn);

  mct::closed_hash_map<Handle, ConnectionInfo*>::iterator a = m_streams->find(connection->getStreamA().getHandle());
  mct::closed_hash_map<Handle, ConnectionInfo*>::iterator b = m_streams->find(connection->getStreamB().getHandle());

  if(connection == NULL)
    throw std::logic_error("null connection in thread stream info");

  try
  {
    if(a == m_streams->end() || b == m_streams->end())
      throw std::logic_error("could not find both sides of thread stream");

    delete connection;
    m_streams->erase(a);
    m_streams->erase(b);
  }
  catch(...)
  {
    delete connection;

    if(a != m_streams->end())
      m_streams->erase(a);

    if(b != m_streams->end())
      m_streams->erase(b);

    throw;
  }
}

void CommRegistry::setMode(bool asynchronous,
                           CommRegistry::CallbackFunction* callbackFunction,
                           Thread* callbackThread)
{
  m_mutex.lock();

  m_callbackFunction = callbackFunction;
  m_callbackThread = callbackThread;

  try
  {
    if(asynchronous && m_internalThread == NULL)
    {
      m_internalThread = new CommThread(*this);
      m_internalThread->start();
    }
    else if(!asynchronous && m_internalThread != NULL)
    {
      delete m_internalThread;
    }
  }
  catch(...)
  {
    m_mutex.unlock();
    throw;
  }

  m_mutex.unlock();
}

void* CommRegistry::accept(StreamType& type, uint32_t timeout)
{
  void* newStream = NULL;

  m_mutex.lock();

  try
  {
    if(m_internalThread != NULL)
      throw std::logic_error("cannot use synchronous accept while in asynchronous mode");

    newStream = acceptWait(type, timeout);
  }
  catch(...)
  {
    m_mutex.unlock();
    throw;
  }

  return newStream;
}

uint32_t CommRegistry::accept(uint32_t count, uint32_t timeout)
{
  uint32_t numStreams = 0;
  uint32_t endTime = getTime() + timeout;
  StreamType type;

  m_mutex.lock(timeout);

  try
  {
    if(m_internalThread != NULL)
      throw std::logic_error("cannot use synchronous accept while in asynchronous mode");

    if(m_callbackFunction == NULL && m_callbackThread == NULL)
      throw std::logic_error("cannot use multiple accept without a callback function or thread for new connections");

    timeout = getTimeout(endTime);

    // Accept new connections on m_pipeIn
    do
    {
      acceptWait(type, timeout);
      ++numStreams;
      timeout = getTimeout(endTime);
    } while(timeout != 0 && numStreams < count);
  }
  catch(...)
  {
    m_mutex.unlock();
    throw;
  }

  m_mutex.unlock();
  return numStreams;
}

void* CommRegistry::acceptWait(StreamType& type, uint32_t timeout)
{
  void* newStream = NULL;
  Handle handle;

  switch(m_waitSet.waitAny(timeout, handle)) // TODO: handle waitAbandoned?
  {
  case WaitSuccess: break;
  case WaitTimeout: return NULL;
  default:
    throw std::runtime_error("error when waiting on CommRegistry waitSet");
  }

  // Create inter-process connection
  if(handle == m_pipeIn.getHandle())
  {
    newStream = receiveProcessStream(type, timeout);
  }
  else
  {
    // Create socket connection
  }

  if(newStream == NULL)
    throw std::runtime_error("failed to create new stream");

  if(type != ProcessByte &&
     type != ProcessMessage &&
     type != SocketByte &&
     type != SocketMessage)
    throw std::runtime_error("invalid type of new stream");

  // Call callbacks if enabled
  if(m_callbackFunction != NULL)
  {
    // If the callback function returns false, we delete the stream
    if(!(*m_callbackFunction)(newStream, type))
    {
      if(type == ProcessByte || type == SocketByte)
        delete reinterpret_cast<ByteStream*>(newStream);
      else if(type == ProcessMessage || type == SocketMessage)
        delete reinterpret_cast<MessageStream*>(newStream);
      else
        throw std::logic_error("invalid stream type");

      return NULL;
    }
  }

  // Get the handle of the new stream
  if(type == ThreadByte || type == ProcessByte || type == SocketByte)
    handle = reinterpret_cast<ByteStream*>(newStream)->getHandle();
  else
    handle = reinterpret_cast<MessageStream*>(newStream)->getHandle();

  ConnectionInfo* info = new ConnectionInfo(type, handle, newStream, NULL);

  // Add the stream to the mapping structures
  m_streams->insert(std::make_pair(handle, info));

  if(m_callbackThread != NULL)
  {
    m_callbackThread->addWaitObject(*reinterpret_cast<WaitObject*>(newStream));
  }

  return newStream;
}

void* CommRegistry::receiveProcessStream(StreamType& type, uint32_t timeout)
{
  StreamInfo info(m_pipeIn);
  void* newStream = NULL;

  switch(info.m_type)
  {
  case StreamType::ProcessByte:
    newStream = new ProcessByteStream(info.m_processId, timeout);
    break;

  case StreamType::ProcessMessage:
    newStream = new ProcessMessageStream(info.m_processId, m_defaultMessageStreamSize, timeout);
    break;

  default:
    throw std::runtime_error("received invalid stream type on incoming CommRegistry pipe");
  }

  type = info.m_type;
  return newStream;
}

uint32_t CommRegistry::getTimeout(uint32_t endTime)
{
  uint32_t currentTime = getTime();
  return ((currentTime > endTime) ? 0 : (endTime - currentTime));
}

CommRegistry::ConnectionInfo::ConnectionInfo(StreamType type,
                                             Handle handle,
                                             void* stream,
                                             void* connection) :
  m_type(type),
  m_handle(handle),
  m_stream(stream),
  m_connection(connection)
{
  // Do nothing
}

CommRegistry::StreamInfo::StreamInfo(StreamType type,
                                     uint32_t processId) :
  m_type(type),
  m_processId(processId)
{
  // Do nothing
}

CommRegistry::StreamInfo::StreamInfo(ByteStream& pipe)
{
  // Read the stream info
  if(pipe.receive(this, sizeof(StreamInfo)) != sizeof(StreamInfo))
    throw std::runtime_error("received incomplete stream info on incoming CommRegistry pipe");
}
