#include "LinuxThread.h"
#include "Abstraction.h"
#include "Exception.h"
#include <errno.h>

LinuxThread::LinuxThread(uint32_t timeout) :
  m_exit(false),
  m_runEvent(false), // Start out not running
  m_pauseEvent(false),
  m_exitedEvent(false),
  m_timeout(timeout)
{
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&m_thread, &attr, threadHook, this);
  pthread_attr_destroy(&attr);

  m_fdSet.add(m_pauseEvent.getHandle());
}

LinuxThread::~LinuxThread()
{
  // Make sure the thread is in the right state
  m_exit = true;
  m_runEvent.set();
  m_pauseEvent.set();

  WaitForObject(m_exitedEvent.getHandle(), INFINITE);
}

void* LinuxThread::threadMain()
{
  try
  {
    int fd;

    while(true)
    {
      WaitForObject(m_runEvent.getHandle(), INFINITE);

      if(m_exit)
        break;

      switch(m_fdSet.waitAny(m_timeout, fd))
      {      
      case WaitSuccess:
        if(fd == m_pauseEvent.getHandle())
        {
          m_pauseEvent.reset();
          break;
        }
      case WaitTimeout:
        iterate(fd);
        break;

      case WaitAbandoned:
        if(fd == m_pauseEvent.getHandle())
          throw Exception("Pause event abandoned");

        abandoned(fd);
        m_fdSet.remove(fd);
        break;

      default:
        throw Exception("Thread internal wait returned unexpected value");
      }
    }
  }
  catch(Exception& ex)
  {
    m_error = ex.what();
  }

  m_exitedEvent.set();
  return NULL;
}

void LinuxThread::start()
{
  // Check if there is a problem with the thread
  if(!m_error.empty()) throw Exception("Thread exited with exception: " + m_error);
  m_pauseEvent.reset();
  m_runEvent.set();
}

void LinuxThread::pause()
{
  // Check if there is a problem with the thread
  if(!m_error.empty()) throw Exception("Thread exited with exception: " + m_error);
  m_runEvent.reset();
  m_pauseEvent.set();
}

void LinuxThread::stop()
{
  // Check if there is a problem with the thread
  if(!m_error.empty()) throw Exception("Thread exited with exception: " + m_error);
  m_exit = true;
  m_runEvent.set();
  m_pauseEvent.set();
}

void* LinuxThread::threadHook(void* param)
{
  return reinterpret_cast<LinuxThread*>(param)->threadMain();
}

void LinuxThread::addWaitObject(int fd)
{
  m_fdSet.add(fd);
}

void LinuxThread::removeWaitObject(int fd)
{
  m_fdSet.remove(fd);
}

void LinuxThread::setWaitTimeout(uint32_t timeout)
{
  m_timeout = timeout;
}

void LinuxThread::abandoned(int fd __attribute__ ((unused)))
{
  // Do nothing, optionally implemented by a derived class
}
