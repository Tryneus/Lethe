#include "windows/WindowsThread.h"
#include <Windows.h>

using namespace lethe;

WindowsThread::WindowsThread(uint32_t timeout) :
  BaseThread(timeout),
#pragma warning(disable:4355) // Suppress warning about using 'this'
  m_handle(CreateThread(NULL, 0, &threadHook, this, 0, NULL))
#pragma warning(default:4355)
{
  // Do nothing
}

WindowsThread::~WindowsThread()
{
  // The thread may not have exited yet, but we have to close it now
  // The BaseThread class will wait on the exited event anyway
  CloseHandle(m_handle);
}

DWORD WINAPI WindowsThread::threadHook(void* param)
{
  reinterpret_cast<WindowsThread*>(param)->threadMain();
  return 0;
}
