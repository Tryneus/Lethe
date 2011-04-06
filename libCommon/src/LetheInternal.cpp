#include "LetheInternal.h"
#include "LetheFunctions.h"

uint32_t lethe::getEndTime(uint32_t timeout)
{
  if(timeout != INFINITE)
    timeout += getTime();
  return timeout;
}

uint32_t lethe::getTimeout(uint32_t endTime)
{
  if(endTime != INFINITE)
  {
    uint32_t currentTime = getTime();
    endTime = ((currentTime > endTime) ? 0 : (endTime - currentTime));
  }

  return endTime;
}

