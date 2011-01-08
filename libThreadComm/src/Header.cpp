#include "Header.h"
#include "Exception.h"

#if defined(_WIN32)
#include <new.h>
#endif

using namespace ThreadComm;

Header::Header(uint32_t size) :
  m_size(size),
  m_dataArea(new char[size]),
  m_receiveList(m_dataArea),
  m_releaseList(m_dataArea + sizeof(Message)),
  m_unallocList(m_dataArea + 2 * sizeof(Message), &m_dataArea[size])
{
  // Initialize buffers
  Message* firstMessage = new (m_dataArea)
    Message(this, sizeof(Message), Message::Nil);

  Message* secondMessage = new (m_dataArea + sizeof(Message))
    Message(this, sizeof(Message), Message::Pend);

  Message* thirdMessage = new (m_dataArea + 2 * sizeof(Message))
    Message(this, m_size - 2 * sizeof(Message), Message::Free);

  secondMessage->setLastOnStack(firstMessage);
  thirdMessage->setLastOnStack(secondMessage);

#if defined(_WIN32)

  m_semaphore = CreateSemaphore(NULL, 0, 2048, NULL);

  if(m_semaphore == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create semaphore");

  m_waitingToSend = false;

#elif defined(__linux__)

  int pipes[2];

  if(pipe(pipes) == -1)
    throw Exception("Failed to create pipe");

  m_pipeIn = pipes[0];
  m_pipeOut = pipes[1];

#endif
}

Header::~Header()
{
#if defined(_WIN32)

  CloseHandle(m_semaphore);

#elif defined(__linux__)

  close(m_pipeIn);
  close(m_pipeOut);

#endif

  delete [] m_dataArea;
}

Message* Header::allocate(uint32_t size)
{
  Message* message = m_releaseList.pop();

  while(message != NULL)
  {
    m_unallocList.unallocate(message);
    message = m_releaseList.pop();
  }

  return m_unallocList.allocate(size);
}

void Header::send(Message* message)
{
#if defined(_WIN32)

  if(m_waitingToSend && !ReleaseSemaphore(m_semaphore, 1, NULL))
    throw Exception("Semaphore full, other side needs to wait on it");
  else
    m_waitingToSend = false;

#endif

  message->setState(Message::Sent);
  m_receiveList.pushBack(message);

#if defined(_WIN32)

  if(!ReleaseSemaphore(m_semaphore, 1, NULL))
    m_waitingToSend = true;

#elif defined(__linux__)

  if(write(m_pipeOut, "S", 1) != 1)
  {
    // TODO: Add handling for a failed write
    throw Exception("Failed to write to the pipe");
  }

#endif
}

Message* Header::receive()
{
#if defined(_WIN32)

  // TODO: anything to do here?  user should call wait

#elif defined(__linux__)

  char buffer[1];
  if(read(m_pipeIn, buffer, 1) != 1)
  {
    // TODO: Add handling for a failed read
    throw Exception("Failed to read from the pipe");
  }

#endif

  Message* extraMessage;
  Message* message = m_receiveList.receive(extraMessage);

  if(extraMessage != NULL)
    m_releaseList.pushBack(extraMessage);

  return message;
}

bool Header::release(Message* message)
{
  if(reinterpret_cast<void*>(message) < reinterpret_cast<void*>(m_dataArea) ||
    reinterpret_cast<void*>(message) > reinterpret_cast<void*>(m_dataArea + m_size))
    return false; // Message didn't belong to this side, but it might belong to the other side

  switch(message->getState())
  {
  case Message::Alloc:
    m_unallocList.unallocate(message);
    break;
  case Message::Recv:
    message->setState(Message::Pend);
    m_releaseList.pushBack(message);
    break;
  case Message::Nil:
    message->setState(Message::Pend);
    break;
  default:
    throw Exception("Attempt to release a message in the wrong state");
  }

  return true;
}
