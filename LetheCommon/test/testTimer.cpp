#include "Lethe.h"
#include "LetheException.h"
#include "testCommon.h"
#include "catch/catch.hpp"

using namespace lethe;

TEST_CASE("timer/structor", "Test construction/destruction")
{
  // Create a lot of mutexes
  const uint32_t numTimers(1000);
  Timer** timerArray = new Timer*[numTimers];

  for(uint32_t i(0); i < numTimers; ++i)
  {
    // Use 'i' to determine the parameters
    timerArray[i] = new Timer(INFINITE, false, false);
  }

  // Put the timers into some different states
  for(uint32_t i(0); i < numTimers; ++i)
  {
    if(i & bit1)
      timerArray[i]->start(2000, false);
    if(i & bit2)
      timerArray[i]->clear();
    if(i & bit3)
      timerArray[i]->start(1000, false);
    if(i & bit4)
      timerArray[i]->clear();
  }

  // Destroy
  for(uint32_t i(0); i < numTimers; ++i)
  {
    delete timerArray[i];
  }

  delete [] timerArray;
}

TEST_CASE("timer/wait", "Test waiting for timers")
{
  Timer timer(INFINITE, false, false);
  uint64_t startTime;
  uint64_t endTime;

  startTime = getTime();
  timer.start(1000, false);
  REQUIRE(WaitForObject(timer, 1050) == WaitSuccess);
  endTime = getTime();
  REQUIRE(endTime - startTime >= 1000);
  REQUIRE(endTime - startTime < 1050);
  REQUIRE(WaitForObject(timer, 0) == WaitSuccess);
  timer.clear();
  REQUIRE(WaitForObject(timer, 0) == WaitTimeout);

  startTime = getTime();
  timer.start(1, false);
  REQUIRE(WaitForObject(timer, 50) == WaitSuccess);
  endTime = getTime();
  REQUIRE(endTime - startTime >= 1);
  REQUIRE(endTime - startTime < 50);
  REQUIRE(WaitForObject(timer, 0) == WaitSuccess);
  timer.clear();
  REQUIRE(WaitForObject(timer, 0) == WaitTimeout);
}
