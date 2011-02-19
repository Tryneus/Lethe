#include "Abstraction.h"
#include "AbstractionException.h"
#include "catch.hpp"

TEST_CASE("functions/sleep", "Test the sleep function")
{
  // Require that sleep be accurate within 35ms (due to OS scheduling/timer resolution and latency)
  uint64_t startTime;

  startTime = getTime();
  Sleep(0);
  REQUIRE(getTime() - startTime <= 35);

  startTime = getTime();
  Sleep(1);
  REQUIRE(getTime() - startTime <= 36);

  startTime = getTime();
  Sleep(100);
  REQUIRE(getTime() - startTime <= 135);

  startTime = getTime();
  Sleep(1000);
  REQUIRE(getTime() - startTime <= 1035);
}

TEST_CASE("functions/getTimeString", "Test getting the current time in string form")
{
  // TODO: implement functions/getTimeString
}

TEST_CASE("functions/wait", "Test waiting for single WaitObjects")
{
  // TODO: implement functions/wait
}
