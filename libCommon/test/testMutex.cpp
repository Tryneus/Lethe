#include "catch.hpp"
#include "Abstraction.h"

TEST_CASE("mutex/structor", "Test construction/destruction")
{
  // Create a lot of mutexes
  const uint32_t numMutexes = 1000;
  Mutex** mutexArray = new Mutex*[numMutexes];

  for(uint32_t i = 0; i < numMutexes; ++i)
  { 
    REQUIRE_NOTHROW(mutexArray[i] = new Mutex((bool)(i % 2)));
  }

  for(uint32_t i = 0; i < numMutexes; ++i)
  {
    REQUIRE_NOTHROW(delete mutexArray[i]);
  }

  delete mutexArray;
}

TEST_CASE("mutex/lock", "Test construction with different parameters")
{
  
}

TEST_CASE("mutex/unlock", "Test construction with different parameters")
{
  
}
