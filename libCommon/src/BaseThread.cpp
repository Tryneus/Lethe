#include "BaseThread.h"
#include "AbstractionFunctions.h"
#include "Exception.h"

BaseThread::BaseThread(uint32_t timeout) :
  WaitObject(INVALID_HANDLE_VALUE),
  m_exit(false),
  m_runEvent(false, false), // Start out not running
  m_pauseEvent(false, true),
  m_exitedEvent(false, false),
  m_timeout(timeout)
{
  setWaitHandle(m_exitedEvent.getHandle());
  m_waitSet.add(m_pauseEvent);
}

BaseThread::~BaseThread()
{
  // Make sure the thread is in the right state
  m_exit = true;
  m_runEvent.set();
  m_pauseEvent.set();

  WaitForObject(m_exitedEvent.getHandle(), INFINITE);
}

void* BaseThread::threadMain()
{
  try
  {
    Handle handle;

    while(true)
    {
      WaitForObject(m_runEvent.getHandle(), INFINITE);

      if(m_exit)
        break;

      switch(m_waitSet.waitAny(m_timeout, handle))
      {
      case WaitSuccess:
        if(handle == m_pauseEvent.getHandle())
          break;

      case WaitTimeout:
        iterate(handle);
        break;

      case WaitAbandoned:
        if(handle == m_pauseEvent.getHandle())
          throw Exception("Pause event abandoned");

        abandoned(handle);
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

  m_exit = true;
  m_exitedEvent.set();
  return NULL;
}

void BaseThread::start()
{
  // Check if there is a problem with the thread
  if(!m_error.empty()) throw Exception("Thread exited with exception: " + m_error);
  m_runEvent.set();
}

void BaseThread::pause()
{
  // Check if there is a problem with the thread
  if(!m_error.empty()) throw Exception("Thread exited with exception: " + m_error);
  m_runEvent.reset();
  m_pauseEvent.set();
}

void BaseThread::stop()
{
  // Check if there is a problem with the thread
  if(!m_error.empty()) throw Exception("Thread exited with exception: " + m_error);
  m_exit = true;
  m_runEvent.set();
  m_pauseEvent.set();
}

bool BaseThread::isStopping() const
{
  return m_exit;
}

const std::string& BaseThread::getError() const
{
  return m_error;
}

void BaseThread::addWaitObject(WaitObject& obj)
{
  m_waitSet.add(obj);
}

void BaseThread::removeWaitObject(WaitObject& obj)
{
  m_waitSet.remove(obj);
}

void BaseThread::setWaitTimeout(uint32_t timeout)
{
  m_timeout = timeout;
}

#if defined(__GNUG__) /* Suppress warning in GCC */
void BaseThread::abandoned(Handle handle __attribute__ ((unused)))
#else
void BaseThread::abandoned(Handle handle)
#endif
{
  // Do nothing, optionally implemented by a derived class
}
