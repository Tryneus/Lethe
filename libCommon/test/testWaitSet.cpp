#include "Abstraction.h"
#include "AbstractionException.h"
#include "catch.hpp"
#include "Log.h"

TEST_CASE("waitSet/structor", "Test constructor/destructor")
{
  // No constructor parameters, so just create a few and see if we crash
  const uint32_t numSets(100);
  WaitSet waitSet[numSets];

  for(uint32_t i(0); i < numSets; ++i)
  {
    REQUIRE(waitSet[i].getSize() == 0);
  }
}

// Create an instantiable thread object (so we can add a thread to the WaitSet)
class WaitSetDummyThread : public Thread
{
public:
  WaitSetDummyThread() : Thread(0) { };
protected:
  void iterate(Handle handle GCC_UNUSED) { stop(); };
};

// Create an error condition with a custom WaitObject
class InvalidWaitObject : public WaitObject
{
public:
  InvalidWaitObject() : WaitObject(INVALID_HANDLE_VALUE) { };
};

TEST_CASE("waitSet/add", "Test adding WaitObjects")
{
  WaitSet waitSet;
  Mutex mutex(false);
  Event event(false, false);
  Timer timer;
  Semaphore semaphore(1, 1);
  WaitSetDummyThread thread;
  Pipe pipe;
  InvalidWaitObject invalid;

  REQUIRE(waitSet.getSize() == 0);

  waitSet.add(mutex);
  REQUIRE(waitSet.getSize() == 1);
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 1);

  waitSet.add(event);
  REQUIRE(waitSet.getSize() == 2);
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 2);

  waitSet.add(timer);
  REQUIRE(waitSet.getSize() == 3);
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 3);

  waitSet.add(semaphore);
  REQUIRE(waitSet.getSize() == 4);
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 4);

  waitSet.add(thread);
  REQUIRE(waitSet.getSize() == 5);
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 5);

  waitSet.add(pipe);
  REQUIRE(waitSet.getSize() == 6);
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 6);

  // Try to add everything twice, expect failures
  REQUIRE_FALSE(waitSet.add(mutex));
  REQUIRE_FALSE(waitSet.add(event));
  REQUIRE_FALSE(waitSet.add(timer));
  REQUIRE_FALSE(waitSet.add(semaphore));
  REQUIRE_FALSE(waitSet.add(thread));
  REQUIRE_FALSE(waitSet.add(pipe));
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 6);
}

TEST_CASE("waitSet/remove", "Test removing WaitObjects")
{
  WaitSet waitSet;
  Mutex mutex(false);
  Event event(false, false);
  Timer timer;
  Semaphore semaphore(1, 1);
  WaitSetDummyThread thread;
  Pipe pipe;
  InvalidWaitObject invalid;

  REQUIRE(waitSet.getSize() == 0);

  waitSet.add(mutex);
  waitSet.add(event);
  waitSet.add(timer);
  waitSet.add(semaphore);
  waitSet.add(thread);
  waitSet.add(pipe);
  REQUIRE(waitSet.getSize() == 6);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 6);

  waitSet.remove(pipe);
  REQUIRE(waitSet.getSize() == 5);
  REQUIRE_FALSE(waitSet.remove(pipe));
  REQUIRE(waitSet.getSize() == 5);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 5);

  waitSet.remove(mutex);
  REQUIRE(waitSet.getSize() == 4);
  REQUIRE_FALSE(waitSet.remove(mutex));
  REQUIRE(waitSet.getSize() == 4);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 4);

  waitSet.remove(semaphore);
  REQUIRE(waitSet.getSize() == 3);
  REQUIRE_FALSE(waitSet.remove(semaphore));
  REQUIRE(waitSet.getSize() == 3);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 3);

  waitSet.remove(event);
  REQUIRE(waitSet.getSize() == 2);
  REQUIRE_FALSE(waitSet.remove(event));
  REQUIRE(waitSet.getSize() == 2);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 2);

  waitSet.remove(thread);
  REQUIRE(waitSet.getSize() == 1);
  REQUIRE_FALSE(waitSet.remove(thread));
  REQUIRE(waitSet.getSize() == 1);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 1);

  waitSet.remove(timer);
  REQUIRE(waitSet.getSize() == 0);
  REQUIRE_FALSE(waitSet.remove(timer));
  REQUIRE(waitSet.getSize() == 0);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 0);
}

TEST_CASE("waitSet/waitAny", "Test waiting for any WaitObjects")
{
  // The waitAny function may throw exceptions, but I can't find a way to make it do so
  //  This would probably require an OS error to happen
  WaitSet waitSet;
  uint8_t buffer[5] = { "test" }; // Buffer for reading/writing on pipe
  Handle waitHandle;

  // Create wait objects initially unsignalled
  Event event(false, true);
  Timer timer;
  Semaphore semaphore(10, 0);
  WaitSetDummyThread thread;
  Pipe pipe;

  REQUIRE(waitSet.getSize() == 0);

  waitSet.add(event);
  waitSet.add(timer);
  waitSet.add(semaphore);
  waitSet.add(thread);
  waitSet.add(pipe);
  REQUIRE(waitSet.getSize() == 5);

  // Trigger each object, make sure we get exactly one at a time
  event.set();
  REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess); // Event is reset here
  REQUIRE(waitHandle == event.getHandle());
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  timer.start(1);
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitSuccess);
  REQUIRE(waitHandle == timer.getHandle());
  timer.clear(); // This may be changed if we support auto-reset in LinuxTimers
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  semaphore.unlock(1);
  REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess); // Semaphore is relocked here
  REQUIRE(waitHandle == semaphore.getHandle());
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  thread.start();
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitSuccess); // WaitSetDummyThread may take a little time to exit
  REQUIRE(waitHandle == thread.getHandle());
  waitSet.remove(thread); // Thread is not resettable at the moment
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  pipe.send(buffer, 5);
  REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess);
  REQUIRE(waitHandle == pipe.getHandle());
  pipe.receive(buffer, 5);
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  // Create a second thread since we used up the first
  WaitSetDummyThread thread2;
  waitSet.add(thread2);

  // Now try triggering all the objects at once and make sure that every object finishes
  thread2.start();
  timer.start(1);
  event.set();
  semaphore.unlock(1);
  pipe.send(buffer, 5);

  // Create a set to track the handles we still need to wait on
  std::set<Handle> unfinished;
  unfinished.insert(thread2.getHandle());
  unfinished.insert(timer.getHandle());
  unfinished.insert(event.getHandle());
  unfinished.insert(semaphore.getHandle());
  unfinished.insert(pipe.getHandle());

  // Sleep a little to make sure the notifications have pushed through
  Sleep(10);

  while(unfinished.size() > 0)
  {
    REQUIRE(waitSet.waitAny(20, waitHandle) == WaitSuccess);
    REQUIRE(unfinished.erase(waitHandle) == 1);
  }
}

/* TODO: Enable this test case once fixed on linux
TEST_CASE("waitSet/waitAny2", "Test behavior of waitAny in different conditions")
{
  WaitSet waitSet;
  WaitSetDummyThread thread;
  Timer timer;
  Mutex mutex(false); // Unlocked
  Event event1(false, false); // Not yet set, manual reset
  Event event2(false, true); // Not yet set, autoreset
  Event event3(true, false); // Already set, manual reset
  Event event4(true, true); // Already set, autoreset
  Semaphore sem1(10, 1); // Already set
  Semaphore sem2(10, 0); // Not yet set
  Pipe pipe;
  Handle waitHandle;
  uint8_t buffer[5]; // Buffer to read out of pipe

  // Set the objects to signalled before adding them to the wait set
  timer.start(1);
  event1.set();
  event2.set();
  sem2.unlock(1);
  pipe.send("text", 5);

  // Add to wait set
  waitSet.add(thread);
  waitSet.add(timer);
  waitSet.add(mutex);
  waitSet.add(event1);
  waitSet.add(event2);
  waitSet.add(event3);
  waitSet.add(event4);
  waitSet.add(sem1);
  waitSet.add(sem2);
  waitSet.add(pipe);

  // Create a set to track the handles we still need to wait on
  std::set<Handle> unfinished;
  unfinished.insert(thread.getHandle());
  unfinished.insert(timer.getHandle());
  unfinished.insert(mutex.getHandle());
  unfinished.insert(event1.getHandle());
  unfinished.insert(event2.getHandle());
  unfinished.insert(event3.getHandle());
  unfinished.insert(event4.getHandle());
  unfinished.insert(sem1.getHandle());
  unfinished.insert(sem2.getHandle());
  unfinished.insert(pipe.getHandle());

  LogInfo("Thread handle: " << thread.getHandle());
  LogInfo("Timer handle: " << timer.getHandle());
  LogInfo("Mutex handle: " << mutex.getHandle());
  LogInfo("Event1 handle: " << event1.getHandle());
  LogInfo("Event2 handle: " << event2.getHandle());
  LogInfo("Event3 handle: " << event3.getHandle());
  LogInfo("Event4 handle: " << event4.getHandle());
  LogInfo("Sem1 handle: " << sem1.getHandle());
  LogInfo("Sem2 handle: " << sem2.getHandle());
  LogInfo("Pipe handle: " << pipe.getHandle());

  // Sleep a bit to make sure the timer finishes
  Sleep(5);

  while(unfinished.size() > 0)
  {
    REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess);

    LogInfo("Wait Success, handle: " << waitHandle);

    // Special handling to reset timer, thread, and pipe handles
    // TODO: remove this, shouldn't be necessary
    if(waitHandle == timer.getHandle())
      timer.clear();
    else if(waitHandle == thread.getHandle())
      waitSet.remove(thread);
    else if(waitHandle == pipe.getHandle())
      pipe.receive(buffer, 5);
    else if(waitHandle == mutex.getHandle())
      waitSet.remove(mutex);
    else if(waitHandle == event1.getHandle())
      event1.reset();
    else if(waitHandle == event3.getHandle())
      event3.reset();

    REQUIRE(unfinished.erase(waitHandle) == 1);
  }
}
*/

// TODO: add test for abandoned (deleted) WaitObjects
