#include "Abstraction.h"
#include "AbstractionException.h"
#include "catch.hpp"
#include "testCommon.h"

TEST_CASE("semaphore/structor", "Test construction/destruction")
{
  // Create a lot of mutexes
  const uint32_t numSemaphores(1000);
  Semaphore** semaphoreArray = new Semaphore*[numSemaphores];

  for(uint32_t i(0); i < numSemaphores; ++i)
  {
    // Use 'i' to determine the parameters
    semaphoreArray[i] = new Semaphore(i + 2, (i & bit1) + 2);
  }

  // Put the Semaphores into some different states
  for(uint32_t i(0); i < numSemaphores; ++i)
  {
    if(i & bit2)
      semaphoreArray[i]->lock();
    if(i & bit3)
      semaphoreArray[i]->unlock(1);
    if(i & bit4)
      semaphoreArray[i]->lock();
    if(i & bit5)
      semaphoreArray[i]->unlock(1);
  }

  // Destroy
  for(uint32_t i(0); i < numSemaphores; ++i)
  {
    delete semaphoreArray[i];
  }

  delete [] semaphoreArray;
}

TEST_CASE("semaphore/limit", "Test semaphore maximum value behavior")
{
  // Semaphore maximum value 1, initial value 0
  Semaphore sem1(1, 0);
  REQUIRE(WaitForObject(sem1, 20) == WaitTimeout);
  REQUIRE_THROWS_AS(sem1.unlock(2), std::bad_syscall);
  sem1.unlock(1);
  REQUIRE_THROWS_AS(sem1.unlock(1), std::bad_syscall);
  REQUIRE(WaitForObject(sem1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem1, 20) == WaitTimeout);
  REQUIRE_THROWS_AS(sem1.unlock(2), std::bad_syscall);
  sem1.unlock(1);
  REQUIRE(WaitForObject(sem1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem1, 20) == WaitTimeout);

  // Semaphore maximum value 1, initial value 1
  Semaphore sem2(1, 1);
  REQUIRE_THROWS_AS(sem2.unlock(1), std::bad_syscall);
  REQUIRE(WaitForObject(sem2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem2, 20) == WaitTimeout);
  REQUIRE_THROWS_AS(sem2.unlock(2), std::bad_syscall);
  sem2.unlock(1);
  REQUIRE_THROWS_AS(sem2.unlock(1), std::bad_syscall);
  REQUIRE(WaitForObject(sem2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem2, 20) == WaitTimeout);

  // Semaphore maximum value 10, initial value 5
  Semaphore sem3(10, 5);
  REQUIRE_THROWS_AS(sem3.unlock(6), std::bad_syscall);
  sem3.unlock(5);
  for(uint32_t i = 0; i < 10; ++i)
    REQUIRE(WaitForObject(sem3, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem3, 20) == WaitTimeout);
  REQUIRE_THROWS_AS(sem3.unlock(11), std::bad_syscall);
  sem3.unlock(10);

  Semaphore sem4(10, 10);
  // TODO: add these

  Semaphore sem5(1000, 100);
}

TEST_CASE("semaphore/lock", "Test semaphore lock behavior")
{
  // TODO: implement semaphore/lock
}

TEST_CASE("semaphore/unlock", "Test semaphore unlock behavior")
{
  // TODO: implement semaphore/unlock
}

