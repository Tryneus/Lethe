#include "WindowsMutex.h"
#include "Exception.h"
#include "Windows.h"
#include <sstream>

WindowsMutex::WindowsMutex(bool locked) :
  m_handle(CreateMutex(NULL, locked, NULL))
{
  if(m_handle == INVALID_HANDLE_VALUE)
  {
    std::stringstream errorText;
    errorText << "Mutex creation failed, last error: " << GetLastError();
    throw Exception(errorText.str());
  }
}

WindowsMutex::~WindowsMutex()
{
  CloseHandle(m_handle);
}

void WindowsMutex::lock()
{
  DWORD result = WaitForSingleObject(m_handle, INFINITE);

  if(result != WAIT_OBJECT_0)
  {
    std::stringstream errorText;
    errorText << "Mutex wait failed, result: " << result
      << ", last error: " << GetLastError();
    throw Exception(errorText.str());
  }
}

void WindowsMutex::unlock()
{
  if(!ReleaseMutex(m_handle))
  {
    std::stringstream errorText;
    errorText << "Mutex wait failed, last error: " << GetLastError();
    throw Exception(errorText.str());
  }
}


