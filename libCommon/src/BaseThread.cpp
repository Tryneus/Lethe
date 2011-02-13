#include "BaseThread.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include <sstream>

BaseThread::BaseThread(uint32_t timeout) :
  WaitObject(INVALID_HANDLE_VALUE),
  m_running(false),
  m_exit(false),
  m_triggerEvent(false, true),
  m_stoppedEvent(false, false),
  m_exitedEvent(false, false),
  m_mutex(false),
  m_timeout(timeout)
{
  setWaitHandle(m_stoppedEvent.getHandle());
  m_waitSet.add(m_triggerEvent);
}

BaseThread::~BaseThread()
{
  m_exit = true;
  stop();
  WaitForObject(m_exitedEvent, INFINITE);
}

void BaseThread::threadMain()
{
  try
  {
    Handle handle;

    do
    {
      WaitForObject(m_triggerEvent, INFINITE);

      if(m_exit)
        break;

      if(!m_running)
        continue;

      m_stoppedEvent.reset();

      do
      {
        if(m_objectQueue.size() > 0)
          handleObjectQueue();

        switch(m_waitSet.waitAny(m_timeout, handle))
        {
        case WaitSuccess:
          if(handle == m_triggerEvent.getHandle())
            break;

        case WaitTimeout:
          iterate(handle);
          break;

        case WaitAbandoned:
          if(handle == m_triggerEvent.getHandle())
            throw std::logic_error("thread trigger event abandoned");

          abandoned(handle);
          break;

        default:
          throw std::logic_error("thread internal wait failed");
        }
      } while(m_running);

      m_stoppedEvent.set();

    } while(!m_exit);
  }
  catch(std::exception& ex)
  {
    m_mutex.lock();
    m_error.assign(ex.what());
    m_mutex.unlock();
  }
  catch(...)
  {
    m_mutex.lock();
    m_error.assign("Unrecognized exception");
    m_mutex.unlock();
  }

  // Make sure all the notifications are set
  m_running = false;
  m_exit = true;
  m_stoppedEvent.set();
  m_exitedEvent.set();
}

void BaseThread::iterate(Handle handle __attribute((unused)))
{
  // Do nothing
}

void BaseThread::start()
{
  // Check if there is a problem with the thread
  m_mutex.lock();
  if(!m_error.empty()) throw std::runtime_error("Thread exited with exception: " + m_error);
  m_mutex.unlock();

  if(m_exit) throw std::logic_error("thread has stopped and cannot be restarted");

  m_running = true;
  m_triggerEvent.set();
}

void BaseThread::stop()
{
  m_running = false;
  m_triggerEvent.set();
}

bool BaseThread::isStopping() const
{
  return !m_running;
}

std::string BaseThread::getError()
{
  m_mutex.lock();
  std::string error(m_error);
  m_mutex.unlock();

  return error;
}

void BaseThread::addWaitObject(WaitObject& obj)
{
  m_mutex.lock();
  m_objectQueue.push(std::pair<bool, WaitObject&>(true, obj));
  m_mutex.unlock();
}

void BaseThread::removeWaitObject(WaitObject& obj)
{
  m_mutex.lock();
  m_objectQueue.push(std::pair<bool, WaitObject&>(false, obj));
  m_mutex.unlock();
}

void BaseThread::handleObjectQueue()
{
  m_mutex.lock();
  while(m_objectQueue.size() > 0)
  {
    if(m_objectQueue.front().first)
      m_waitSet.add(m_objectQueue.front().second);
    else
      m_waitSet.remove(m_objectQueue.front().second);

    m_objectQueue.pop();
  }
  m_mutex.unlock();
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
