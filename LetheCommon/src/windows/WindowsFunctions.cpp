#include "LetheTypes.h"
#include "LetheBasic.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

uint32_t getParentProcessId()
{
  Handle toolhelp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  uint32_t processId = GetCurrentProcessId();
  PROCESSENTRY32 processInfo;

  if(toolhelp == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("CreateToolhelp32Snapshot", lastError());

  processInfo.dwSize = sizeof(PROCESSENTRY32);

  if(!Process32First(toolhelp, &process))
  {
    CloseHandle(toolhelp);
    throw std::runtime_error("parent process id not found");
  }

  while(processInfo.th32ProcessID != processId)
  {
    if(!Process32Next(toolhelp, &processInfo))
    {
      CloseHandle(toolhelp);
      throw std::runtime_error("parent process id not found");
    }
  }
  
  CloseHandle(toolhelp);

  return processInfo.th32ProcessID;
}

uint32_t lethe::createProcess(const std::string& command,
                              const std::vector<std::string>& arguments,
                              const std::vector<std::pair<std::string, std::string> >& environment)
{
  std::vector<std::string> envConverted;
  PROCESS_INFORMATION processInfo;
  STARTUPINFO startupInfo;
  uint32_t commandLength = command.length() + 1;
  uint32_t envLength = 1;
  char* commandString;
  char* envString;

  // Get the length required for the environment string
  for(size_t i = 0; i < environment.size(); ++i)
    envLength += environment[i].first.length() + environment[i].second.length() + 2;

  envString = new char[envLength];
  envLength = 0;

  // Copy the environment strings over
  for(size_t i = 0; i < environment.size(); ++i)
  {
    memcpy(&envString[envLength], environment[i].first.c_str(), environment[i].first.length());
    envLength += environment[i].first.length();

    envString[envLength++] = '=';

    memcpy(&envString[envLength], environment[i].second.c_str(), environment[i].second.length() + 1);
    envLength += environment[i].second.length() + 1;
  }

  envString[envLength] = '\0';

  // Get the length required for the command string
  for(size_t i = 0; i < arguments.size(); ++i)
    commandLength += arguments[i].length() + 1;

  commandString = new char[commandLength];

  memcpy(&commandString[0], command.c_str(), command.length());
  commandLength = command.length();

  // Copy the arguments over
  for(size_t i = 0; i < arguments.size(); ++i)
  {
    commandString[commandLength++] = ' ';

    memcpy(&commandString[commandLength], arguments[i].c_str(), arguments[i].length());
    commandLength += arguments[i].length();
  }

  commandString[commandLength] = '\0';

  memset(&startupInfo, 0, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);

  bool result = CreateProcess("", command.c_str(), NULL, NULL, false, 0, envString, &startupInfo, &processInfo);

  delete [] envString;
  delete [] commandString;

  if(!result)
    throw std::bad_syscall("CreateProcess", lastError());

  CloseHandle(processInfo.hProcess);
  CloseHandle(processInfo.hThread);

  return processInfo.dwProcessId;
}

std::ostream& lethe::operator << (std::ostream& out, const lethe::Handle& handle)
{
  std::ios_base::fmtflags flags = out.flags(std::ios_base::hex);

  if(sizeof(handle) == sizeof(uint32_t))
    out << "0x" << (uint32_t)handle;
  else if(sizeof(handle) == sizeof(uint64_t))
    out << "0x" << (uint32_t)handle;
  else
    out << "<unexpected handle length>";

  out.flags(flags);
  return out;
}

void lethe::sleep_ms(uint32_t timeout)
{
  Sleep(timeout);
}

std::string lethe::getErrorString(uint32_t errorCode)
{
  TCHAR* buffer(NULL);

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_MAX_WIDTH_MASK,
                NULL,
                errorCode,
                0,
                (LPTSTR)&buffer,
                0,
                NULL);

  std::string retval(buffer);

  LocalFree(buffer);

  return retval;
}

std::string lethe::lastError()
{
  return lethe::getErrorString(GetLastError());
}

uint64_t lethe::getTime()
{
  const uint64_t msPerYear = (uint64_t)(60 * 60 * 24) * 365240;
  const uint64_t unixEpochDelta = 369 * msPerYear;
  FILETIME currentTime;

  GetSystemTimeAsFileTime(&currentTime);

  uint64_t retval = ((uint64_t)currentTime.dwHighDateTime << 32) + currentTime.dwLowDateTime;

  // Adjust for scale: 100ns units to 1ms units
  retval /= 10000;

  // Adjust for the number of milliseconds difference between 1601 and 1970 (windows to unix epoch);
  retval -= unixEpochDelta;
  return retval;
}

std::string lethe::getTimeString()
{
  std::stringstream timeString;
  SYSTEMTIME sysTime;

  GetLocalTime(&sysTime);

  switch(sysTime.wMonth)
  {
  case 1:  timeString << "Jan "; break;
  case 2:  timeString << "Feb "; break;
  case 3:  timeString << "Mar "; break;
  case 4:  timeString << "Apr "; break;
  case 5:  timeString << "May "; break;
  case 6:  timeString << "Jun "; break;
  case 7:  timeString << "Jul "; break;
  case 8:  timeString << "Aug "; break;
  case 9:  timeString << "Sep "; break;
  case 10: timeString << "Oct "; break;
  case 11: timeString << "Nov "; break;
  case 12: timeString << "Dec "; break;
  default: timeString << sysTime.wMonth << " "; break;
  }

  timeString << std::setfill('0');
  timeString << sysTime.wDay    << " "
    << sysTime.wHour   << ":"
    << std::setw(2) << sysTime.wMinute << ":"
    << std::setw(2) << sysTime.wSecond << "."
    << std::setw(3) << sysTime.wMilliseconds;

  return timeString.str();
}

uint32_t lethe::getProcessId()
{
  return static_cast<uint32_t>(GetCurrentProcessId());
}

uint32_t lethe::getThreadId()
{
  return static_cast<uint32_t>(GetCurrentThreadId());
}

uint32_t lethe::seedRandom(uint32_t seed)
{
  if(seed == 0)
    seed = GetTickCount();

  srand(seed);

  return seed;
}

lethe::WaitResult lethe::WaitForObject(lethe::WaitObject& obj, uint32_t timeout)
{
  switch(WaitForSingleObject(obj.getHandle(), timeout))
  {
  case WAIT_OBJECT_0:
    return lethe::WaitSuccess;

  case WAIT_ABANDONED_0:
    return lethe::WaitAbandoned;

  case WAIT_TIMEOUT:
    return lethe::WaitTimeout;

  default:
    throw std::bad_syscall("WaitForSingleObject", lethe::lastError());
  }
}

