#include "Abstraction.h"
#include "Exception.h"
#include "catch.hpp"
#include "testCommon.h"

TEST_CASE("timer/structor", "Test construction/destruction")
{
  // Create a lot of mutexes
  const uint32_t numTimers(1000);
  Timer** timerArray = new Timer*[numTimers];

  for(uint32_t i(0); i < numTimers; ++i)
  {
    // Use 'i' to determine the parameters
    timerArray[i] = new Timer();
  }

  // Put the timers into some different states
  for(uint32_t i(0); i < numTimers; ++i)
  {
    if(i & bit1)
      timerArray[i]->start(2000);
    if(i & bit2)
      timerArray[i]->clear();
    if(i & bit3)
      timerArray[i]->start(1000);
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
  Timer timer;

  timer.start(1000);
  REQUIRE(WaitForObject(timer, 1000) == WaitSuccess);
  REQUIRE(WaitForObject(timer, 0) == WaitSuccess);
  timer.clear();
  REQUIRE(WaitForObject(timer, 0) == WaitTimeout);

  timer.start(1);
  REQUIRE(WaitForObject(timer, 1) == WaitSuccess);
  REQUIRE(WaitForObject(timer, 0) == WaitSuccess);
  timer.clear();
  REQUIRE(WaitForObject(timer, 0) == WaitTimeout);
}
