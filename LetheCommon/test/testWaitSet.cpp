#include "Lethe.h"
#include "LetheException.h"
#include "LetheInternal.h"
#include "Log.h"
#include "catch/catch.hpp"

using namespace lethe;

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
  WaitSetDummyThread() : Thread(INFINITE) { };
protected:
  void iterate(Handle handle GCC_UNUSED) { };
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
  Timer timer(INFINITE);
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
  Timer timer(INFINITE);
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
  Timer timer(INFINITE);
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

  // Thread starts out signalled, start it to clear that
  thread.start();

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

  thread.stop();
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitSuccess); // WaitSetDummyThread may take a little time to exit
  REQUIRE(waitHandle == thread.getHandle());
  thread.start(); // Resets thread
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  pipe.send(buffer, 5);
  REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess);
  REQUIRE(waitHandle == pipe.getHandle());
  pipe.receive(buffer, 5);
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  // Now try triggering all the objects at once and make sure that every object finishes
  thread.stop();
  timer.start(1);
  event.set();
  semaphore.unlock(1);
  pipe.send(buffer, 5);

  // Create a set to track the handles we still need to wait on
  std::set<Handle> unfinished;
  unfinished.insert(thread.getHandle());
  unfinished.insert(timer.getHandle());
  unfinished.insert(event.getHandle());
  unfinished.insert(semaphore.getHandle());
  unfinished.insert(pipe.getHandle());

  // Sleep a little to make sure the notifications have pushed through
  sleep_ms(10);

  while(!unfinished.empty())
  {
    REQUIRE(waitSet.waitAny(20, waitHandle) == WaitSuccess);
    REQUIRE(unfinished.erase(waitHandle) == 1);
  }

  // Clear manual reset objects and make sure wait times out
  thread.start();
  timer.clear();
  pipe.receive(buffer, 5);

  REQUIRE(waitSet.waitAny(20, waitHandle) == WaitTimeout);
  REQUIRE(waitHandle == INVALID_HANDLE_VALUE);
}

TEST_CASE("waitSet/waitAny2", "Test behavior of waitAny in different conditions")
{
  WaitSet waitSet;
  WaitSetDummyThread thread;
  Timer timer(INFINITE);
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

  // Sleep a bit to make sure the timer finishes
  sleep_ms(5);

  while(!unfinished.empty())
  {
    REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess);
    REQUIRE(unfinished.erase(waitHandle) == 1);
  }

  // Clear persistent objects, make sure wait fails
  event1.reset();
  event3.reset();
  timer.clear();
  thread.start();
  waitSet.remove(mutex);
  pipe.receive(buffer, 5);

  REQUIRE(waitSet.waitAny(20, waitHandle) == WaitTimeout);
  REQUIRE(waitHandle == INVALID_HANDLE_VALUE);
}

TEST_CASE("waitSet/abandoned", "Test WaitSet behavior with abandoned objects")
{
  // Construct two sets to try slightly different situations
  WaitSet waitSet;
  WaitSet waitSet2;
  std::set<Handle> unfinished;
  std::set<Handle> unfinished2;

  // Have an object of every type in different states
  Mutex mutex1(false);
  Mutex mutex2(true);
  Event event1(false, false);
  Event event2(false, true);
  Event event3(true, false);
  Event event4(true, true);
  WaitSetDummyThread thread1;
  WaitSetDummyThread thread2;
  Timer timer1(INFINITE);
  Timer timer2(INFINITE);
  Timer timer3(INFINITE);
  Pipe pipe1;
  Pipe pipe2;
  Semaphore sem1(10, 0);
  Semaphore sem2(10, 5);
  Semaphore sem3(10, 10);

  // Create some different states (if not done at construction)
  thread2.start();
  timer2.start(1);
  timer3.start(10000);
  pipe2.send("text", 5);

  // Add WaitObject handles to the sets
  unfinished.insert(mutex1.getHandle());
  unfinished.insert(mutex2.getHandle());
  unfinished.insert(event1.getHandle());
  unfinished.insert(event2.getHandle());
  unfinished.insert(event3.getHandle());
  unfinished.insert(event4.getHandle());
  unfinished.insert(thread1.getHandle());
  unfinished.insert(thread2.getHandle());
  unfinished.insert(timer1.getHandle());
  unfinished.insert(timer2.getHandle());
  unfinished.insert(timer3.getHandle());
  unfinished.insert(pipe1.getHandle());
  unfinished.insert(pipe2.getHandle());
  unfinished.insert(sem1.getHandle());
  unfinished.insert(sem2.getHandle());
  unfinished.insert(sem3.getHandle());

  unfinished2 = unfinished;

  waitSet.add(mutex1);
  waitSet.add(mutex2);
  waitSet.add(event1);
  waitSet.add(event2);
  waitSet.add(event3);
  waitSet.add(event4);
  waitSet.add(thread1);
  waitSet.add(thread2);
  waitSet.add(timer1);
  waitSet.add(timer2);
  waitSet.add(timer3);
  waitSet.add(pipe1);
  waitSet.add(pipe2);
  waitSet.add(sem1);
  waitSet.add(sem2);
  waitSet.add(sem3);

  waitSet2.add(mutex1);
  waitSet2.add(mutex2);
  waitSet2.add(event1);
  waitSet2.add(event2);
  waitSet2.add(event3);
  waitSet2.add(event4);
  waitSet2.add(thread1);
  waitSet2.add(thread2);
  waitSet2.add(timer1);
  waitSet2.add(timer2);
  waitSet2.add(timer3);
  waitSet2.add(pipe1);
  waitSet2.add(pipe2);
  waitSet2.add(sem1);
  waitSet2.add(sem2);
  waitSet2.add(sem3);

#if defined(__linux__)
  // Can't destroy objects because of callbacks, instead close handles manually
  for(std::set<Handle>::const_iterator i = unfinished.begin(); i != unfinished.end(); ++i)

    close(*i);

  Handle waitHandle;

  while(!unfinished.empty())
  {
    REQUIRE(waitSet.waitAny(0, waitHandle) == WaitAbandoned);
    REQUIRE(unfinished.erase(waitHandle) == 1);
    REQUIRE(waitSet.remove(waitHandle));
  }

  // Make sure that all abandoned events are still received (without duplicates),
  //  even if the wait object is not removed.  This will prove if fair load
  //  balancing is done in the wait set on abandoned handles
  while(!unfinished2.empty())
  {
    REQUIRE(waitSet2.waitAny(0, waitHandle) == WaitAbandoned);
    REQUIRE(unfinished2.erase(waitHandle) == 1);
  }
#elif defined(__WIN32__) || defined(_WIN32)
  // TODO: figure out how to force abandoned wait result in windows
#endif
}



