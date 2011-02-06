#include "Abstraction.h"
#include "Exception.h"
#include "catch.hpp"

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
class DummyThread : public Thread
{
protected:
  void iterate(Handle handle) { handle = INVALID_HANDLE_VALUE; };
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
  DummyThread thread;
  Pipe pipe;
  InvalidWaitObject invalid;

  REQUIRE(waitSet.getSize() == 0);

  waitSet.add(mutex);
  REQUIRE(waitSet.getSize() == 1);
  REQUIRE_THROWS_AS(waitSet.add(invalid), Exception);
  REQUIRE(waitSet.getSize() == 1);

  waitSet.add(event);
  REQUIRE(waitSet.getSize() == 2);
  REQUIRE_THROWS_AS(waitSet.add(invalid), Exception);
  REQUIRE(waitSet.getSize() == 2);

  waitSet.add(timer);
  REQUIRE(waitSet.getSize() == 3);
  REQUIRE_THROWS_AS(waitSet.add(invalid), Exception);
  REQUIRE(waitSet.getSize() == 3);

  waitSet.add(semaphore);
  REQUIRE(waitSet.getSize() == 4);
  REQUIRE_THROWS_AS(waitSet.add(invalid), Exception);
  REQUIRE(waitSet.getSize() == 4);

  waitSet.add(thread);
  REQUIRE(waitSet.getSize() == 5);
  REQUIRE_THROWS_AS(waitSet.add(invalid), Exception);
  REQUIRE(waitSet.getSize() == 5);

  waitSet.add(pipe);
  REQUIRE(waitSet.getSize() == 6);
  REQUIRE_THROWS_AS(waitSet.add(invalid), Exception);
  REQUIRE(waitSet.getSize() == 6);

  // Try to add everything twice, expect exceptions
  REQUIRE_FALSE(waitSet.add(mutex));
  REQUIRE_FALSE(waitSet.add(event));
  REQUIRE_FALSE(waitSet.add(timer));
  REQUIRE_FALSE(waitSet.add(semaphore));
  REQUIRE_FALSE(waitSet.add(thread));
  REQUIRE_FALSE(waitSet.add(pipe));
  REQUIRE_THROWS_AS(waitSet.add(invalid), Exception);
  REQUIRE(waitSet.getSize() == 6); 
}

TEST_CASE("waitSet/remove", "Test removing WaitObjects")
{
  WaitSet waitSet;
  Mutex mutex(false);
  Event event(false, false);
  Timer timer;
  Semaphore semaphore(1, 1);
  DummyThread thread;
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
  REQUIRE_THROWS_AS(waitSet.remove(invalid), Exception);
  REQUIRE(waitSet.getSize() == 6); 

  waitSet.remove(pipe);
  REQUIRE(waitSet.getSize() == 5);
  REQUIRE_FALSE(waitSet.remove(pipe));
  REQUIRE(waitSet.getSize() == 5);
  REQUIRE_THROWS_AS(waitSet.remove(invalid), Exception);
  REQUIRE(waitSet.getSize() == 5); 

  waitSet.remove(mutex);
  REQUIRE(waitSet.getSize() == 4);
  REQUIRE_FALSE(waitSet.remove(mutex));
  REQUIRE(waitSet.getSize() == 4);
  REQUIRE_THROWS_AS(waitSet.remove(invalid), Exception);
  REQUIRE(waitSet.getSize() == 4); 

  waitSet.remove(semaphore);
  REQUIRE(waitSet.getSize() == 3);
  REQUIRE_FALSE(waitSet.remove(semaphore));
  REQUIRE(waitSet.getSize() == 3);
  REQUIRE_THROWS_AS(waitSet.remove(invalid), Exception);
  REQUIRE(waitSet.getSize() == 3); 

  waitSet.remove(event);
  REQUIRE(waitSet.getSize() == 2);
  REQUIRE_FALSE(waitSet.remove(event));
  REQUIRE(waitSet.getSize() == 2);
  REQUIRE_THROWS_AS(waitSet.remove(invalid), Exception);
  REQUIRE(waitSet.getSize() == 2); 

  waitSet.remove(thread);
  REQUIRE(waitSet.getSize() == 1);
  REQUIRE_FALSE(waitSet.remove(thread));
  REQUIRE(waitSet.getSize() == 1);
  REQUIRE_THROWS_AS(waitSet.remove(invalid), Exception);
  REQUIRE(waitSet.getSize() == 1); 

  waitSet.remove(timer);
  REQUIRE(waitSet.getSize() == 0);
  REQUIRE_FALSE(waitSet.remove(timer));
  REQUIRE(waitSet.getSize() == 0);
  REQUIRE_THROWS_AS(waitSet.remove(invalid), Exception);
  REQUIRE(waitSet.getSize() == 0); 
}

TEST_CASE("waitSet/waitAll", "Test waiting for all WaitObjects")
{
  // TODO: implement waitSet/waitAll
}

TEST_CASE("waitSet/waitAny", "Test waiting for any WaitObjects")
{
  // The waitAny function may throw exceptions, but I can't find a way to make it do so
  //  This would probably require an OS error to happen
  WaitSet waitSet;
  uint8_t buffer[5] = { "test" }; // Buffer for reading/writing on pipe
  Handle waitHandle;

  // Create wait objects initially unsignalled
  Mutex mutex(true);
  Event event(false, true);
  Timer timer;
  Semaphore semaphore(10, 0);
  DummyThread thread;
  Pipe pipe;

  REQUIRE(waitSet.getSize() == 0);

  waitSet.add(mutex);
  waitSet.add(event);
  waitSet.add(timer);
  waitSet.add(semaphore);
  waitSet.add(thread);
  waitSet.add(pipe);
  REQUIRE(waitSet.getSize() == 6);

  // Trigger each object, make sure we get exactly one at a time
  mutex.unlock();
//  if(waitSet.waitAny(0, waitHandle) != WaitSuccess)
//    REQUIRE(false);
  //  REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess); // Mutex is relocked here

  REQUIRE(WaitSuccess == waitSet.waitAny(0, waitHandle));
  REQUIRE(waitHandle == mutex.getHandle());
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  event.set();
  REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess); // Event is reset here
  REQUIRE(waitHandle == event.getHandle());
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  timer.start(50);
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitSuccess);
  REQUIRE(waitHandle == timer.getHandle());
  timer.clear(); // This may be changed if we support auto-reset in LinuxTimers
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  semaphore.unlock(1);
  REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess); // Semaphore is relocked here
  REQUIRE(waitHandle == semaphore.getHandle());
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  thread.start();
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitSuccess); // DummyThread may take a little time to exit
  REQUIRE(waitHandle == thread.getHandle());
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  pipe.send(buffer, 5);
  REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess);
  REQUIRE(waitHandle == pipe.getHandle());
  pipe.receive(buffer, 5);
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  // Now try triggering all the objects at once and make sure that every object finishes
  thread.start();
  timer.start(1);
  mutex.unlock();
  event.set();
  semaphore.unlock(1);
  pipe.send(buffer, 5);

  // Create a set to track the handles we still need to wait on
  std::set<Handle> unfinished;
  unfinished.insert(thread.getHandle());
  unfinished.insert(timer.getHandle());
  unfinished.insert(mutex.getHandle());
  unfinished.insert(event.getHandle());
  unfinished.insert(semaphore.getHandle());
  unfinished.insert(pipe.getHandle());

  while(unfinished.size() > 0)
  {
    REQUIRE(waitSet.waitAny(10, waitHandle) == WaitSuccess);
    REQUIRE(unfinished.erase(waitHandle) == 1);

    // Special handling to reset timer and pipe handles
    if(waitHandle == timer.getHandle())
      timer.clear();

    if(waitHandle == pipe.getHandle())
      pipe.receive(buffer, 5);
  }
}
