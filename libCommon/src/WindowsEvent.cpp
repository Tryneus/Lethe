#include "WindowsEvent.h"
#include "Exception.h"
#include "Windows.h"
#include <sstream>

WindowsEvent::WindowsEvent(bool initialState) :
  m_handle(CreateEvent(NULL, true, initialState, NULL))
{
  if(m_handle == INVALID_HANDLE_VALUE)
  {
    std::stringstream errorText;
    errorText << "Event creation failed, last error: " << GetLastError();
    throw Exception(errorText.str());
  }
}

WindowsEvent::~WindowsEvent()
{
  CloseHandle(m_handle);
}

HANDLE WindowsEvent::getHandle() const
{
  return m_handle;
}

void WindowsEvent::set()
{
  if(!SetEvent(m_handle))
  {
    std::stringstream errorText;
    errorText << "Set event failed, last error: " << GetLastError();
    throw Exception(errorText.str());
  }
}

void WindowsEvent::reset()
{
  if(!ResetEvent(m_handle))
  {
    std::stringstream errorText;
    errorText << "Reset event failed, last error: " << GetLastError();
    throw Exception(errorText.str());
  }
}
