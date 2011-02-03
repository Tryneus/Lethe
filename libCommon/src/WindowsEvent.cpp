#include "windows/WindowsEvent.h"
#include "Exception.h"
#include <Windows.h>

WindowsEvent::WindowsEvent(bool initialState, bool autoReset) :
  m_handle(CreateEvent(NULL, !autoReset, initialState, NULL))
{
  if(m_handle == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create event: " + lastError());
}

WindowsEvent::~WindowsEvent()
{
  if(!CloseHandle(m_handle))
    throw Exception("Failed to close handle: " + lastError());
}

HANDLE WindowsEvent::getHandle() const
{
  return m_handle;
}

void WindowsEvent::set()
{
  if(!SetEvent(m_handle))
    throw Exception("Failed to set event: " + lastError());
}

void WindowsEvent::reset()
{
  if(!ResetEvent(m_handle))
    throw Exception("Failed to reset event: " + lastError());
}
