#include "LinuxHandleTransfer.h"
#include <sys/socket.h>
#include <sys/un.h>

using namespace lethe;

const std::string LinuxHandleTransfer::s_udsPath("/tmp/lethe/");
const std::string LinuxHandleTransfer::s_udsBaseName("lethe-uds-");

LinuxHandleTransfer::LinuxHandleTransfer(ByteStream& stream,
                                         const std::string& name,
                                         bool serverSide,
                                         uint32_t timeout) :
  m_name(s_udsPath + s_udsBaseName + name),
  m_socket(INVALID_HANDLE_VALUE)
{
  Handle tempSocket;
  struct sockaddr_un address;
  socklen_t addressLength = sizeof(address.sun_family) + m_name.length() + 1;
  char buffer = '\0';

  // Make sure the uds path exists
  if(mkdir(s_udsPath.c_str(), 0777) != 0 && errno != EEXIST) // TODO: security concerns
    throw std::bad_syscall("mkdir", lastError());

  tempSocket = socket(PF_UNIX, SOCK_STREAM, 0);

  address.sun_family = AF_UNIX;
  m_name.copy(address.sun_path, std::string::npos, 0);
  address.sun_path[m_name.length()] = '\0';

  if(serverSide)
  {
    if(tempSocket == INVALID_HANDLE_VALUE)
      throw std::bad_syscall("socket", lastError());

    unlink(m_name.c_str());

    try
    {
      if(bind(tempSocket, (sockaddr*) &address, addressLength) != 0)
        throw std::bad_syscall("bind", lastError());

      if(listen(tempSocket, 1) != 0)
        throw std::bad_syscall("listen", lastError());

      if(fcntl(tempSocket, F_SETFL, O_NONBLOCK) != 0)
        throw std::bad_syscall("fcntl", lastError());

      stream.send(&buffer, 1); // Tell the other side the uds is ready

      pollfd event;
      event.fd = tempSocket;
      event.events = POLLIN;

      if(poll(&event, 1, timeout) != 1 || !(event.revents & POLLIN))
        throw std::bad_syscall("poll", lastError());

      m_socket = accept(tempSocket, (sockaddr*) &address, &addressLength);

      if(m_socket < 0)
        throw std::bad_syscall("accept", lastError());
    }
    catch(...)
    {
      close(tempSocket);
      unlink(m_name.c_str());
      throw;
    }

    close(tempSocket);
  }
  else
  {
    if(WaitForObject(stream, timeout) != WaitSuccess)
      throw std::runtime_error("timed out waiting for uds");

    if(stream.receive(&buffer, 1) != 1 || buffer != '\0')
      throw std::logic_error("incorrect data in stream");

    if(connect(tempSocket, (sockaddr*) &address, addressLength) != 0)
    {
      std::string errorString = lastError();
      close(tempSocket);
      throw std::bad_syscall("connect", errorString);
    }
    m_socket = tempSocket;
  }
}

LinuxHandleTransfer::~LinuxHandleTransfer()
{
  close(m_socket);
  unlink(m_name.c_str());
}

void LinuxHandleTransfer::sendTimer(const Timer& timer)
{
  sendInternal(timer.getHandle(), s_timerType);
}

void LinuxHandleTransfer::sendEvent(const Event& event)
{
  sendInternal(event.getHandle(), s_eventType);
}

void LinuxHandleTransfer::sendSemaphore(const Semaphore& semaphore)
{
  sendInternal(semaphore.getHandle(), s_semaphoreType);
}

Timer* LinuxHandleTransfer::recvTimer()
{
  return new Timer(recvInternal(s_timerType));
}

Event* LinuxHandleTransfer::recvEvent()
{
  return new Event(recvInternal(s_eventType));
}

Semaphore* LinuxHandleTransfer::recvSemaphore()
{
  return new Semaphore(recvInternal(s_semaphoreType));
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

Handle LinuxHandleTransfer::recvInternal(char handleType)
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

  if(poll(&event, 1, 1000) != 1 || !(event.revents & POLLIN))
    throw std::bad_syscall("poll", lastError());

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
