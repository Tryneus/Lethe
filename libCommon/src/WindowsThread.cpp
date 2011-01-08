#include <exception>
#include "WindowsThread.h"
#include "Abstraction.h"
#include "Exception.h"
#include "Windows.h"

WindowsThread::WindowsThread(uint32_t timeout) :
  m_exit(false),
  m_runEvent(false), // Start out not running
  m_pauseEvent(false), // Pause event will be set when the thread is paused
  m_timeout(timeout),
#pragma warning(disable:4355)
  m_handle(CreateThread(NULL, 0, threadHook, this, 0, NULL))
#pragma warning(default:4355)
{
  m_handleSet.add(m_pauseEvent.getHandle());
}

WindowsThread::~WindowsThread()
{
  // Make sure the thread is in the right state
  m_exit = true;
  m_runEvent.set();
  m_pauseEvent.set();

  if(m_handle != INVALID_HANDLE_VALUE)
    WaitForObject(m_handle, INFINITE);
}

DWORD WindowsThread::threadMain()
{
  try
  {
    HANDLE handle;

    while(WaitForObject(m_runEvent.getHandle(), INFINITE) != WaitError)
    {
      if(m_exit)
        break;

      switch(m_handleSet.waitAny(m_timeout, handle))
      {      
      case WaitSuccess:
        if(handle == m_pauseEvent.getHandle())
        {
          m_pauseEvent.reset();
          break;
        }
      case WaitTimeout:
        iterate(handle);
        break;

      case WaitAbandoned:
        if(handle == m_pauseEvent.getHandle())
          throw Exception("Pause event abandoned");

        abandoned(handle);
        m_handleSet.remove(handle);
        break;

      case WaitError:
        throw Exception("Thread internal wait failed");
      default:
        throw Exception("Thread internal wait returned unexpected value");
      }
    }
  }
  catch(Exception& ex)
  {
    m_error = ex.what();
  }

  return 0;
}

void WindowsThread::start()
{
  // Check if there is a problem with the thread
  if(!m_error.empty()) throw Exception("Thread exited with exception: " + m_error);
  m_pauseEvent.reset();
  m_runEvent.set();
}

void WindowsThread::pause()
{
  // Check if there is a problem with the thread
  if(!m_error.empty()) throw Exception("Thread exited with exception: " + m_error);
  m_runEvent.reset();
  m_pauseEvent.set();
}

void WindowsThread::stop()
{
  // Check if there is a problem with the thread
  if(!m_error.empty()) throw Exception("Thread exited with exception: " + m_error);
  m_exit = true;
  m_runEvent.set();
  m_pauseEvent.set();
}

DWORD WINAPI WindowsThread::threadHook(void* param)
{
  return reinterpret_cast<WindowsThread*>(param)->threadMain();
}

bool WindowsThread::addWaitObject(HANDLE handle)
{
  return m_handleSet.add(handle);
}

bool WindowsThread::removeWaitObject(HANDLE handle)
{
  return m_handleSet.remove(handle);
}

void WindowsThread::setWaitTimeout(uint32_t timeout)
{
  m_timeout = timeout;
}

void WindowsThread::abandoned(HANDLE handle)
{
  // Do nothing, optionally implemented by a derived class
}