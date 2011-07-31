#include "LetheInternal.h"
#include "LetheFunctions.h"

uint64_t lethe::getEndTime(uint32_t timeout)
{
  uint64_t endTime = timeout;

  if(timeout != INFINITE)
    endTime = timeout + getTime();

  return endTime;
}

uint64_t lethe::getTimeout(uint64_t endTime)
{
  uint64_t timeout = endTime;

  if(endTime != INFINITE)
  {
    uint64_t currentTime = getTime();
    timeout = ((currentTime > endTime) ? 0 : (endTime - currentTime));
  }

  return timeout;
}

