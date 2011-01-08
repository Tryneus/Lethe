#include "LinuxMutex.h"
#include "Exception.h"
#include <poll.h>

LinuxMutex::LinuxMutex(bool locked) :
  m_lockingThread(pthread_self())
{
  int pipes[2];

  if(pipe(pipes) != 0)
    throw Exception("Failed to create mutex pipe");

  m_pipeRead = pipes[0];
  m_pipeWrite = pipes[1];

  if(!locked)
    unlock();
}

LinuxMutex::~LinuxMutex()
{
  close(m_pipeRead);
  close(m_pipeWrite);
}

void LinuxMutex::lock(int timeout)
{
  struct pollfd mutex;
  char buffer[1];

  if(m_lockingThread == pthread_self())
    return;

  mutex.fd = m_pipeRead;
  mutex.events = POLLIN;

  switch(poll(&mutex, 1, timeout))
  {
  case 1:
    if(mutex.revents & (POLLERR | POLLHUP | POLLNVAL))
      throw Exception("Error on the mutex pipe");

    if(read(m_pipeRead, buffer, 1) != 1)
      throw Exception("Problem reading from the pipe");

    m_lockingThread = pthread_self();
    break;
  case 0:
    throw Exception("Mutex lock timed out");
  default:
    throw Exception("Mutex lock failed");
  }
}

void LinuxMutex::unlock()
{
  if(pthread_self() != m_lockingThread)
    throw Exception("Mutex can only be unlocked by the thread that locked it");

  m_lockingThread = -1;

  if(write(m_pipeWrite, "U", 1) != 1)
    throw Exception("Failed to write to the mutex pipe");
}


