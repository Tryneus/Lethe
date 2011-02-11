#include "AbstractionTypes.h"
#include "AbstractionBasic.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

std::string lastError()
{
  TCHAR* buffer(NULL);

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_MAX_WIDTH_MASK,
                NULL,
                GetLastError(),
                0,
                (LPTSTR)&buffer,
                0,
                NULL);

  std::string retval(buffer);

  LocalFree(buffer);

  return retval;
}

void getFileList(const std::string& directory,
                 std::vector<std::string>& fileList)
{
  WIN32_FIND_DATA fileData;
  Handle findHandle(FindFirstFile(directory.c_str(), &fileData));

  fileList.clear();

  if(findHandle != INVALID_HANDLE_VALUE)
  {
    do
    {
      fileList.push_back(fileData.cFileName);
    } while(FindNextFile(findHandle, &fileData));

    FindClose(findHandle);
  }
}

uint64_t getTime()
{
  const uint64_t msPerYear = 60 * 60 * 24 * 365240;
  const uint64_t unixEpochDelta = 369 * msPerYear;
  FILETIME currentTime;

  GetSystemTimeAsFileTime(currentTime);

  uint64_t retval = (currentTime.dwHighDateTime << 32) + currentTime.dwLowDateTime;

  // Adjust for scale: 100ns units to 1ms units
  retval /= 10000;

  // Adjust for the number of milliseconds difference between 1601 and 1970 (windows to unix epoch);
  retval -= unixEpochDelta;
  return retval;
}

std::string getTimeString()
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

uint32_t seedRandom(uint32_t seed)
{
  if(seed == 0)
    seed = GetTickCount();

  srand(seed);

  return seed;
}

WaitResult WaitForObject(WaitObject& obj, uint32_t timeout)
{
  if(obj.preWaitCallback())
    return WaitSuccess;

  switch(WaitForSingleObject(obj.getHandle(), timeout))
  {
  case WAIT_OBJECT_0:
    obj.postWaitCallback(WaitSuccess);
    return WaitSuccess;

  case WAIT_ABANDONED_0:
    obj.postWaitCallback(WaitAbandoned);
    return WaitAbandoned;

  case WAIT_TIMEOUT:
    return WaitTimeout;

  default:
    throw Exception("Failed to wait: " + lastError());
  }
}

