#include "Abstraction.h"
#include "AbstractionException.h"
#include "catch.hpp"
#include "testCommon.h"
#include <stdio.h>

// TestThreadDummy thread loops until manually stop()ed
class TestThreadDummyThread : public Thread
{
public:
  TestThreadDummyThread() : Thread(0) { };
  ~TestThreadDummyThread() { };

protected:
  void iterate(Handle handle GCC_UNUSED) { /* Do nothing */ };
  void abandoned(Handle handle GCC_UNUSED) { throw std::logic_error("TestThreadDummyThread received abandoned event"); };
};

// StopThread stops itself after the given timeout
class StopThread : public Thread
{
public:
  StopThread(uint32_t timeout) : Thread(timeout) { };
  ~StopThread() { };

protected:
  void iterate(Handle handle GCC_UNUSED) { stop(); };
  void abandoned(Handle handle GCC_UNUSED) { throw std::logic_error("StopThread received abandoned event"); };
};

// WaitThread waits until the given object triggers, then stops
class WaitThread : public Thread
{
public:
  WaitThread(WaitObject& obj) : Thread(INFINITE) { addWaitObject(obj); };
  ~WaitThread() { };

protected:
  void iterate(Handle handle GCC_UNUSED) { stop(); };
  void abandoned(Handle handle GCC_UNUSED) { throw std::logic_error("WaitThread received abandoned event"); };
};

TEST_CASE("thread/structor", "Test construction/destruction")
{
  uint64_t startTime;

  for(uint32_t i = 0; i < 256; ++i)
  {
    // Create one of each type of thread
    Thread* dThread = new TestThreadDummyThread();
    Thread* sThread = new StopThread(10);
    Thread* wThread = new WaitThread(*dThread);

    if(i & bit1)
      dThread->start();
    if(i & bit2)
      sThread->start();
    if(i & bit3)
      wThread->start();

    if(i & bit4)
      dThread->stop();
    if(i & bit5)
      sThread->stop();
    if(i & bit6)
      wThread->stop();

    REQUIRE(dThread->getError() == "");
    REQUIRE(sThread->getError() == "");
    REQUIRE(wThread->getError() == "");

    // Make sure the threads destruct successfully in a timely fashion
    startTime = getTime();
    delete sThread;
    dThread->stop();
    delete wThread;
    delete dThread;
    // This is set high because the first run is rather slow with valgrind
    REQUIRE(getTime() - startTime < 400);
  }
}

class TestRunThread : public Thread
{
private:
  WaitObject& m_primary;
  WaitObject& m_secondary;
  uint32_t m_primaryIterations;
  uint32_t m_secondaryIterations;

public:
  TestRunThread(WaitObject& primary, WaitObject& secondary) :
    Thread(INFINITE),
    m_primary(primary),
    m_secondary(secondary),
    m_primaryIterations(0),
    m_secondaryIterations(0)
 {
   addWaitObject(primary);
   addWaitObject(secondary);
 }

  uint32_t getPrimaryIterations() { return m_primaryIterations; };
  uint32_t getSecondaryIterations() { return m_secondaryIterations; };

protected:
  void iterate(Handle handle)
  {
    if(handle == m_primary.getHandle()) ++m_primaryIterations;
    else if(handle == m_secondary.getHandle()) ++m_secondaryIterations;
    else throw std::logic_error("incorrect handle");
  }
};

TEST_CASE("thread/run", "Test running threads")
{
  for(uint32_t i = 0; i < 50; ++i)
  {
    Semaphore sem1(100, 25);
    Semaphore sem2(110, 10);
    TestRunThread thread(sem1, sem2);

    thread.start();

    for(uint32_t j = 0; j < 10; ++j)
      sem2.unlock(5);

    for(uint32_t j = 0; j < 50; ++j)
    {
      sem1.unlock(1);
      sem2.unlock(1);
    }

    Sleep(40);

    REQUIRE(thread.getError() == "");
    REQUIRE(thread.getPrimaryIterations() == 75);
    REQUIRE(thread.getSecondaryIterations() == 110);
  }
}

class ExceptionThread : public Thread
{
public:
  ExceptionThread() : Thread(0) { };

protected:
  void iterate(Handle handle GCC_UNUSED) { throw std::logic_error("exception thread");  };
};

TEST_CASE("thread/exception", "Test thread exception handling")
{
  ExceptionThread thread;

  REQUIRE(thread.isStopping() == true);
  REQUIRE(thread.getError() == "");

  thread.start();

  Sleep(1000);

  REQUIRE(thread.isStopping() == true);
  REQUIRE(thread.getError() == "exception thread");

  try
  {
    thread.start();
    FAIL("Starting a dead thread should throw an exception");
  }
  catch(std::runtime_error& ex)
  {
    REQUIRE(std::string(ex.what()) == "thread exited with exception: exception thread");
  }

  thread.stop();

  try
  {
    thread.start();
    FAIL("Starting a dead thread should throw an exception");
  }
  catch(std::runtime_error& ex)
  {
    REQUIRE(std::string(ex.what()) == "thread exited with exception: exception thread");
  }
}

TEST_CASE("thread/timeout", "Test thread iterate timeout")
{

}

class TestStopThread : public Thread
{
private:
  WaitObject& m_trigger;
  uint32_t m_iterationCount;

public:
  TestStopThread(WaitObject& trigger) :
    Thread(INFINITE),
    m_trigger(trigger),
    m_iterationCount(0)
  {
    addWaitObject(m_trigger);
  }

protected:
  void iterate(Handle handle)
  {
    ++m_iterationCount;

    if(m_iterationCount == 1)
      Sleep(1000);
    else if(m_iterationCount == 2)
      stop();
    else if(m_iterationCount == 3)
      setWaitTimeout(300);

    if(handle != m_trigger.getHandle())
      throw std::runtime_error("Shutting down thread");
  }
};

TEST_CASE("thread/stop", "Test stopping threads")
{
  Event event(false, true);
  uint32_t startTime;
  TestStopThread thread(event);

  REQUIRE(thread.isStopping()); // Thread should start out stopped
  thread.stop();
  REQUIRE(thread.isStopping()); // Nothing should have changed yet

  // Iteration 0
  thread.start(); // Thread won't do anything until we give it an event
  REQUIRE_FALSE(thread.isStopping());
  thread.stop();
  REQUIRE(thread.isStopping());
  REQUIRE(thread.getError() == "");

  // Iteration 1
  startTime = getTime();
  thread.start();
  REQUIRE_FALSE(thread.isStopping());
  Sleep(20); // TODO: workaround for waitset problem with autoreset - see TODO.txt
  event.set(); // Thread should now iterate, it will sleep 1 second the first time
  Sleep(20);
  thread.stop(); // Stop the thread, even though it's blocked in the iterate loop
  REQUIRE(thread.isStopping());
  REQUIRE(WaitForObject(thread, 1000) == WaitSuccess); // Wait for the thread to stop
  REQUIRE(thread.getError() == "");
  REQUIRE(getTime() - startTime >= 1000);
  REQUIRE(thread.isStopping());

  // Iteration 2
  event.set();
  thread.start(); // Thread should stop itself as soon as it completes an iteration
  REQUIRE(WaitForObject(thread, 200) == WaitSuccess);
  REQUIRE(thread.getError() == "");
  REQUIRE(thread.isStopping());

  // Iteration 3
  event.set();
  thread.start(); // Thread should iterate once, then again 300ms later and kill itself
  REQUIRE(WaitForObject(thread, 500) == WaitSuccess);
  REQUIRE(thread.isStopping());
  REQUIRE(thread.getError() == "Shutting down thread");
}
