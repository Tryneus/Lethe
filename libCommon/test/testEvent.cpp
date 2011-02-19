#include "Abstraction.h"
#include "AbstractionException.h"
#include "catch.hpp"
#include "testCommon.h"

TEST_CASE("event/structor", "Test construction/destruction")
{
  // Create a lot of mutexes
  const uint32_t numEvents(1000);
  Event** eventArray = new Event*[numEvents];

  for(uint32_t i(0); i < numEvents; ++i)
  {
    // Use 'i' to determine the parameters
    eventArray[i] = new Event(!(i & bit1), !(i & bit2));
  }

  // Put the events into some different states
  for(uint32_t i(0); i < numEvents; ++i)
  {
    if(i & bit3)
      eventArray[i]->set();
    if(i & bit4)
      eventArray[i]->reset();
    if(i & bit5)
      eventArray[i]->set();
    if(i & bit6)
      eventArray[i]->reset();
  }

  // Destroy
  for(uint32_t i(0); i < numEvents; ++i)
  {
    delete eventArray[i];
  }

  delete [] eventArray;
}

TEST_CASE("event/set", "Test setting and normal resetting of events")
{
  // Test an event constructed as set
  Event event1(true, false);
  REQUIRE(WaitForObject(event1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event1, 0) == WaitSuccess);

  // Set it again and make sure nothing breaks
  event1.set();
  REQUIRE(WaitForObject(event1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event1, 0) == WaitSuccess);

  event1.reset();
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);

  event1.reset();
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);

  event1.set();
  REQUIRE(WaitForObject(event1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event1, 0) == WaitSuccess);

  // Test an event constructed as not set
  Event event2(false, false);

  event2.reset();
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);

  event2.set();
  REQUIRE(WaitForObject(event2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event2, 0) == WaitSuccess);

  event2.reset();
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);

  event2.reset();
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);

  event2.set();
  REQUIRE(WaitForObject(event2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event2, 0) == WaitSuccess);
}

TEST_CASE("event/autoreset", "Test auto-reset of events")
{
  // Test an auto-reset event constructed as set
  Event event1(true, true);
  REQUIRE(WaitForObject(event1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);

  event1.set();
  REQUIRE(WaitForObject(event1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);

  event1.reset();
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);

  event1.reset();
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);

  event1.set();
  REQUIRE(WaitForObject(event1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);

  // Test an auto-reset event constructed as not set
  Event event2(false, true);

  event2.reset();
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);

  event2.set();
  REQUIRE(WaitForObject(event2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);

  event2.reset();
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);

  event2.reset();
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);

  event2.set();
  REQUIRE(WaitForObject(event2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);
}
