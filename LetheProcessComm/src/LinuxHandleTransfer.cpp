#include "LetheInternal.h"
#include "LinuxHandleTransfer.h"
#include "TempProcessStream.h"
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

using namespace lethe;

const std::string LinuxHandleTransfer::s_udsPath("/tmp/lethe/");
const std::string LinuxHandleTransfer::s_udsBaseName("lethe-uds-");
Atomic32 LinuxHandleTransfer::s_uniqueId(0);

LinuxHandleTransfer::LinuxHandleTransfer(uint32_t remoteProcessId,
                                         uint32_t timeout) :
  m_socket(INVALID_HANDLE_VALUE),
  m_endTime(getEndTime(timeout))
{
  TempProcessStream stream(remoteProcessId);
  socklen_t addrLength;
  sockaddr_un addr;
  bool serverSide;

  try
  {
    // Get the socket name and figure out if this thread is going to be the server
    serverSide = determineServer(stream);

    // Make sure the uds path exists
    if(mkdir(s_udsPath.c_str(), 0777) != 0 && errno != EEXIST)
      throw std::bad_syscall("mkdir", lastError());

    m_socket = socket(PF_UNIX, SOCK_STREAM, 0);

    if(m_socket == INVALID_HANDLE_VALUE)
      throw std::bad_syscall("socket", lastError());

    if(fcntl(m_socket, F_SETFL, O_NONBLOCK) != 0)
      throw std::bad_syscall("fcntl", lastError());

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, m_name.c_str());
    addrLength = sizeof(addr.sun_family) + m_name.length() + 1;

    if(serverSide)
      initializeServer(stream, addr, addrLength);
    else
      initializeClient(stream, addr, addrLength);
  }
  catch(...)
  {
    if(m_socket != INVALID_HANDLE_VALUE)
      close(m_socket);

    unlink(m_name.c_str());
    throw;
  }
}

LinuxHandleTransfer::LinuxHandleTransfer(ByteStream& stream,
                                         uint32_t timeout) :
  m_socket(INVALID_HANDLE_VALUE),
  m_endTime(getEndTime(timeout))
{
  socklen_t addrLength;
  sockaddr_un addr;
  bool serverSide;

  // Get the socket name and figure out if this thread is going to be the server
  serverSide = determineServer(stream);

  // Make sure the uds path exists
  if(mkdir(s_udsPath.c_str(), 0777) != 0 && errno != EEXIST)
    throw std::bad_syscall("mkdir", lastError());

  try
  {
    m_socket = socket(PF_UNIX, SOCK_STREAM, 0);

    if(m_socket == INVALID_HANDLE_VALUE)
      throw std::bad_syscall("socket", lastError());

    if(fcntl(m_socket, F_SETFL, O_NONBLOCK) != 0)
      throw std::bad_syscall("fcntl", lastError());

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, m_name.c_str());
    addrLength = sizeof(addr.sun_family) + m_name.length() + 1;

    if(serverSide)
      initializeServer(stream, addr, addrLength);
    else
      initializeClient(stream, addr, addrLength);
  }
  catch(...)
  {
    if(m_socket != INVALID_HANDLE_VALUE)
      close(m_socket);

    unlink(m_name.c_str());
    throw;
  }
}

LinuxHandleTransfer::~LinuxHandleTransfer()
{
  close(m_socket);
  unlink(m_name.c_str());
}

bool LinuxHandleTransfer::determineServer(ByteStream& stream)
{
  std::stringstream udsName;
  uint64_t localId = ((uint64_t)getProcessId() << 32) | s_uniqueId.increment();
  uint64_t remoteId;

  // Synchronize with the other side to decide who is the server
  stream.send(&localId, sizeof(localId));

  if(WaitForObject(stream, getTimeout(m_endTime)) != WaitSuccess)
    throw std::runtime_error("LinuxHandleTransfer could not synchronize with remote side");

  if(stream.receive(&remoteId, sizeof(remoteId)) != sizeof(remoteId))
    throw std::runtime_error("LinuxHandleTransfer did not receive correct info from remote side");

  if(localId < remoteId)
    udsName << s_udsPath + s_udsBaseName << ((localId >> 32) & 0xFFFFFFFF) << "-" << (localId & 0xFFFFFFFF);
  else
    udsName << s_udsPath + s_udsBaseName << ((remoteId >> 32) & 0xFFFFFFFF) << "-" << (remoteId & 0xFFFFFFFF);

  m_name.assign(udsName.str());
  return (localId < remoteId);
}

void LinuxHandleTransfer::initializeServer(ByteStream& stream, sockaddr_un& addr, socklen_t addrLength)
{
  char buffer = '\0';
  Handle tempSocket;
  pollfd event;

  if(m_socket == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("socket", lastError());

  // Make sure the chosen file doesn't already exist
  unlink(m_name.c_str());

  if(bind(m_socket, (sockaddr*) &addr, addrLength) != 0)
    throw std::bad_syscall("bind", lastError());

  if(listen(m_socket, 1) != 0)
    throw std::bad_syscall("listen", lastError());

  stream.send(&buffer, 1); // Tell the other side the uds is ready

  event.fd = m_socket;
  event.events = POLLIN;
  event.revents = 0;

  if(poll(&event, 1, getTimeout(m_endTime)) == -1)
    throw std::bad_syscall("poll", lastError());

  if(!(event.revents & POLLIN))
  {
    if(event.revents == 0)
      throw std::runtime_error("timeout while waiting for remote connect");
    else
      throw std::runtime_error("error on socket while waiting for remote connect");
  }

  tempSocket = accept(m_socket, (sockaddr*) &addr, &addrLength);

  if(tempSocket < 0)
    throw std::bad_syscall("accept", lastError());

  close(m_socket);
  m_socket = tempSocket;
}

void LinuxHandleTransfer::initializeClient(ByteStream& stream, sockaddr_un& addr, socklen_t addrLength)
{
  char buffer;

  if(WaitForObject(stream, getTimeout(m_endTime)) != WaitSuccess)
    throw std::runtime_error("timed out waiting for uds");

  if(stream.receive(&buffer, 1) != 1 || buffer != '\0')
    throw std::logic_error("incorrect data in stream");

  if(connect(m_socket, (sockaddr*) &addr, addrLength) != 0)
    throw std::bad_syscall("connect", lastError());
}

void LinuxHandleTransfer::sendPipe(const Pipe& pipe)
{
  // LinuxHandleTransfer has private access to Pipe, both handles are sent
  sendInternal(pipe.m_pipeWrite, s_pipeType);
  sendInternal(pipe.m_pipeRead, s_pipeType);
}

void LinuxHandleTransfer::sendTimer(const Timer& timer)
{
  sendInternal(timer.getHandle(), s_timerType);
}

void LinuxHandleTransfer::sendEvent(const Event& event)
{
  sendInternal(event.getHandle(), s_eventType);
}

void LinuxHandleTransfer::sendMutex(const Mutex& mutex)
{
  sendInternal(mutex.getHandle(), s_mutexType);
}

void LinuxHandleTransfer::sendSemaphore(const Semaphore& semaphore)
{
  sendInternal(semaphore.getHandle(), s_semaphoreType);
}

Pipe* LinuxHandleTransfer::recvPipe(uint32_t timeout)
{
  uint32_t endTime = getEndTime(timeout);
  Handle pipeIn = recvInternal(s_pipeType, timeout);

  timeout = getTimeout(endTime);
  Handle pipeOut = recvInternal(s_pipeType, timeout);

  return new Pipe(pipeIn, pipeOut);
}

Timer* LinuxHandleTransfer::recvTimer(uint32_t timeout)
{
  return new Timer(recvInternal(s_timerType, timeout));
}

Event* LinuxHandleTransfer::recvEvent(uint32_t timeout)
{
  return new Event(recvInternal(s_eventType, timeout));
}

Mutex* LinuxHandleTransfer::recvMutex(uint32_t timeout)
{
  return new Mutex(recvInternal(s_mutexType, timeout));
}

Semaphore* LinuxHandleTransfer::recvSemaphore(uint32_t timeout)
{
  return new Semaphore(recvInternal(s_semaphoreType, timeout));
}

void LinuxHandleTransfer::sendInternal(Handle handle, char handleType)
{
  struct msghdr msg;
  struct iovec iov[1];
  struct cmsghdr *cmsg = NULL;
  char buffer[CMSG_SPACE(sizeof(int))];
  char data[1];

  memset(&msg, 0, sizeof(struct msghdr));
  memset(buffer, 0, CMSG_SPACE(sizeof(int)));

  data[0] = handleType;
  iov[0].iov_base = data;
  iov[0].iov_len = 1;

  msg.msg_iov = iov;
  msg.msg_iovlen = 1;

  msg.msg_control = buffer;
  msg.msg_controllen = sizeof(buffer);

  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof(int));

  *(int*)CMSG_DATA(cmsg) = handle;

  msg.msg_controllen = cmsg->cmsg_len;

  if(sendmsg(m_socket, &msg, 0) < 0)
    throw std::bad_syscall("sendmsg", lastError());
}

Handle LinuxHandleTransfer::recvInternal(char handleType, uint32_t timeout)
{
  struct msghdr msg;
  struct iovec iov[1];
  struct cmsghdr *cmsg = NULL;
  char buffer[CMSG_SPACE(sizeof(int))];
  char data[1];

  memset(&msg, 0, sizeof(struct msghdr));
  memset(buffer, 0, CMSG_SPACE(sizeof(int)));

  iov[0].iov_base = data;
  iov[0].iov_len = 1;

  msg.msg_iov = iov;
  msg.msg_iovlen = 1;

  msg.msg_control = buffer;
  msg.msg_controllen = sizeof(buffer);

  pollfd event;
  event.fd = m_socket;
  event.events = POLLIN;
  event.revents = 0;

  if(poll(&event, 1, timeout) == -1)
    throw std::bad_syscall("poll", lastError());

  if(!(event.revents & POLLIN))
  {
    if(event.revents == 0)
      throw std::runtime_error("timed out waiting for handle");
    else
      throw std::runtime_error("error on socket while receiving handle");
  }

  if(recvmsg(m_socket, &msg, 0) < 0)
    throw std::bad_syscall("recvmsg", lastError());

  for(cmsg = CMSG_FIRSTHDR(&msg);
      cmsg != NULL;
      cmsg = CMSG_NXTHDR(&msg, cmsg))
    if((cmsg->cmsg_level == SOL_SOCKET) &&
       (cmsg->cmsg_type == SCM_RIGHTS))
    {
      if(data[0] != handleType)
        throw std::logic_error("wrong type of handle received");
      return *(int*)CMSG_DATA(cmsg);
    }

  throw std::runtime_error("handle not received");
}
