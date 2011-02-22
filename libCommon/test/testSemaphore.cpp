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

  // Semaphore maximum value 10, initial value 10
  Semaphore sem4(10, 10);
  REQUIRE_THROWS_AS(sem4.unlock(1), std::bad_syscall);
  for(uint32_t i = 0; i < 10; ++i)
    REQUIRE(WaitForObject(sem4, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem4, 20) == WaitTimeout);
  REQUIRE_THROWS_AS(sem4.unlock(11), std::bad_syscall);
  sem4.unlock(10);
  for(uint32_t i = 0; i < 10; ++i)
    REQUIRE(WaitForObject(sem4, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem4, 20) == WaitTimeout);

  // Semaphore maximum value 1000, initial value 100
  Semaphore sem5(1000, 100);
  REQUIRE_THROWS_AS(sem5.unlock(901), std::bad_syscall);
  sem5.unlock(900);
  for(uint32_t i = 0; i < 1000; ++i)
    REQUIRE(WaitForObject(sem5, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem5, 20) == WaitTimeout);
  REQUIRE_THROWS_AS(sem5.unlock(1001), std::bad_syscall);
  sem5.unlock(1000);
  for(uint32_t i = 0; i < 1000; ++i)
    REQUIRE(WaitForObject(sem5, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem5, 20) == WaitTimeout);
}

TEST_CASE("semaphore/waitSet", "Test semaphore using waitSets")
{
  WaitSet waitSet;
  Handle handle;

  // Semaphore maximum value 1, initial value 0
  Semaphore sem1(1, 0);
  waitSet.add(sem1);
  REQUIRE(waitSet.waitAny(0, handle) == WaitTimeout);
  REQUIRE_THROWS_AS(sem1.unlock(2), std::bad_syscall);
  sem1.unlock(1);
  REQUIRE_THROWS_AS(sem1.unlock(1), std::bad_syscall);
  REQUIRE(waitSet.waitAny(0, handle) == WaitSuccess);
  REQUIRE(handle == sem1.getHandle());
  REQUIRE(waitSet.waitAny(20, handle) == WaitTimeout);
  REQUIRE_THROWS_AS(sem1.unlock(2), std::bad_syscall);
  sem1.unlock(1);
  REQUIRE(waitSet.waitAny(0, handle) == WaitSuccess);
  REQUIRE(handle == sem1.getHandle());
  REQUIRE(waitSet.waitAny(20, handle) == WaitTimeout);

  // Semaphore maximum value 1, initial value 1
  Semaphore sem2(1, 1);
  waitSet.add(sem2);
  REQUIRE_THROWS_AS(sem2.unlock(1), std::bad_syscall);
  REQUIRE(waitSet.waitAny(0, handle) == WaitSuccess);
  REQUIRE(handle == sem2.getHandle());
  REQUIRE(waitSet.waitAny(20, handle) == WaitTimeout);
  REQUIRE_THROWS_AS(sem2.unlock(2), std::bad_syscall);
  sem2.unlock(1);
  REQUIRE_THROWS_AS(sem2.unlock(1), std::bad_syscall);
  REQUIRE(waitSet.waitAny(0, handle) == WaitSuccess);
  REQUIRE(handle == sem2.getHandle());
  REQUIRE(waitSet.waitAny(20, handle) == WaitTimeout);

  // Semaphore maximum value 10, initial value 5
  Semaphore sem3(10, 5);
  waitSet.add(sem3);
  REQUIRE_THROWS_AS(sem3.unlock(6), std::bad_syscall);
  sem3.unlock(5);
  for(uint32_t i = 0; i < 10; ++i)
  {
    REQUIRE(waitSet.waitAny(0, handle) == WaitSuccess);
    REQUIRE(handle == sem3.getHandle());
  }
  REQUIRE(waitSet.waitAny(20, handle) == WaitTimeout);
  REQUIRE_THROWS_AS(sem3.unlock(11), std::bad_syscall);

  // Semaphore maximum value 10, initial value 10
  Semaphore sem4(10, 10);
  waitSet.add(sem4);
  REQUIRE_THROWS_AS(sem4.unlock(1), std::bad_syscall);
  for(uint32_t i = 0; i < 10; ++i)
  {
    REQUIRE(waitSet.waitAny(0, handle) == WaitSuccess);
    REQUIRE(handle == sem4.getHandle());
  }
  REQUIRE(waitSet.waitAny(20, handle) == WaitTimeout);
  REQUIRE_THROWS_AS(sem4.unlock(11), std::bad_syscall);
  sem4.unlock(10);
  for(uint32_t i = 0; i < 10; ++i)
  {
    REQUIRE(waitSet.waitAny(0, handle) == WaitSuccess);
    REQUIRE(handle == sem4.getHandle());
  }
  REQUIRE(waitSet.waitAny(20, handle) == WaitTimeout);

  // Semaphore maximum value 1000, initial value 100
  Semaphore sem5(1000, 100);
  waitSet.add(sem5);
  REQUIRE_THROWS_AS(sem5.unlock(901), std::bad_syscall);
  sem5.unlock(900);
  for(uint32_t i = 0; i < 1000; ++i)
  {
    REQUIRE(waitSet.waitAny(0, handle) == WaitSuccess);
    REQUIRE(handle == sem5.getHandle());
  }
  REQUIRE(waitSet.waitAny(20, handle) == WaitTimeout);
  REQUIRE_THROWS_AS(sem5.unlock(1001), std::bad_syscall);
  sem5.unlock(1000);
  for(uint32_t i = 0; i < 1000; ++i)
  {
    REQUIRE(waitSet.waitAny(0, handle) == WaitSuccess);
    REQUIRE(handle == sem5.getHandle());
  }
  REQUIRE(waitSet.waitAny(20, handle) == WaitTimeout);
}

