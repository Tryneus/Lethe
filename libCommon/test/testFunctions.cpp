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

class TestFunctionsThread : public Thread
{
private:
  Mutex& m_mutex;

public:
  TestFunctionsThread(Mutex& mutex) : Thread(0), m_mutex(mutex) { };

protected:
  void iterate(Handle handle GCC_UNUSED) { m_mutex.lock(); stop(); };
};

TEST_CASE("functions/wait", "Test waiting for single WaitObjects")
{
  // Event initially unset, manual reset
  Event event1(false, false);
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);
  event1.set();
  REQUIRE(WaitForObject(event1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event1, 0) == WaitSuccess);
  event1.reset();
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event1, 20) == WaitTimeout);

  // Event initially set, manual reset
  Event event2(true, false);
  REQUIRE(WaitForObject(event2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event2, 0) == WaitSuccess);
  event2.reset();
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event2, 20) == WaitTimeout);
  event2.set();
  REQUIRE(WaitForObject(event2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event2, 0) == WaitSuccess);

  // Event initially unset, automatic reset
  Event event3(false, true);
  REQUIRE(WaitForObject(event3, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event3, 20) == WaitTimeout);
  event3.set();
  REQUIRE(WaitForObject(event3, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event3, 20) == WaitTimeout);
  event3.reset();
  REQUIRE(WaitForObject(event3, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event3, 20) == WaitTimeout);

  // Event initially set, automatic reset
  Event event4(true, true);
  REQUIRE(WaitForObject(event4, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event4, 20) == WaitTimeout);
  event4.reset();
  REQUIRE(WaitForObject(event4, 20) == WaitTimeout);
  REQUIRE(WaitForObject(event4, 20) == WaitTimeout);
  event4.set();
  REQUIRE(WaitForObject(event4, 0) == WaitSuccess);
  REQUIRE(WaitForObject(event4, 20) == WaitTimeout);

  // Mutex initially unlocked
  Mutex mutex1(false);
  REQUIRE(WaitForObject(mutex1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(mutex1, 0) == WaitSuccess);

  // Mutex initially locked by this thread
  Mutex mutex2(true);
  REQUIRE(WaitForObject(mutex2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(mutex2, 0) == WaitSuccess);
  mutex2.unlock();
  mutex2.unlock();
  mutex2.unlock();

  // Mutex locked by another thread
  TestFunctionsThread mutexThread(mutex2);
  mutexThread.start();
  REQUIRE(WaitForObject(mutexThread, 20) == WaitSuccess);
  REQUIRE(WaitForObject(mutex2, 20) == WaitTimeout);
  REQUIRE(WaitForObject(mutex2, 20) == WaitTimeout);

  // Semaphore initially empty
  Semaphore sem1(3, 0);
  REQUIRE(WaitForObject(sem1, 20) == WaitTimeout);
  REQUIRE(WaitForObject(sem1, 20) == WaitTimeout);
  sem1.unlock(3);
  REQUIRE(WaitForObject(sem1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem1, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem1, 20) == WaitTimeout);
  REQUIRE(WaitForObject(sem1, 20) == WaitTimeout);

  // Semaphore initially full
  Semaphore sem2(3, 3);
  REQUIRE(WaitForObject(sem2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem2, 20) == WaitTimeout);
  REQUIRE(WaitForObject(sem2, 20) == WaitTimeout);
  sem2.unlock(3);
  REQUIRE(WaitForObject(sem2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem2, 0) == WaitSuccess);
  REQUIRE(WaitForObject(sem2, 20) == WaitTimeout);

  // Pipe
  Pipe pipe;
  uint8_t buffer[5];
  REQUIRE(WaitForObject(pipe, 20) == WaitTimeout);
  REQUIRE(WaitForObject(pipe, 20) == WaitTimeout);
  pipe.send("test", 5);
  REQUIRE(WaitForObject(pipe, 0) == WaitSuccess);
  REQUIRE(WaitForObject(pipe, 0) == WaitSuccess);
  pipe.receive(buffer, 5);
  REQUIRE(WaitForObject(pipe, 20) == WaitTimeout);
  REQUIRE(WaitForObject(pipe, 20) == WaitTimeout);

  // Thread
  // TODO: add thread test  

  // Timer
  Timer timer;
  REQUIRE(WaitForObject(timer, 20) == WaitTimeout);
  REQUIRE(WaitForObject(timer, 20) == WaitTimeout);
  timer.start(1);
  REQUIRE(WaitForObject(timer, 20) == WaitSuccess);
  REQUIRE(WaitForObject(timer, 0) == WaitSuccess);
  timer.clear();
  REQUIRE(WaitForObject(timer, 20) == WaitTimeout);
  REQUIRE(WaitForObject(timer, 20) == WaitTimeout);
}
