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
    eventArray[i] = new Event(i & bit1, i & bit2);
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

TEST_CASE("event/set", "Test setting of events")
{
  // TODO: implement event/set
}

TEST_CASE("event/reset", "Test resetting of events")
{
  // TODO: implement event/reset
}

TEST_CASE("event/autoreset", "Test auto-reset of events")
{
  // TODO: implement event/autoreset
}
