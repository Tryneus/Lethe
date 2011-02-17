#include "Abstraction.h"
#include "AbstractionException.h"
#include "catch.hpp"
#include "testCommon.h"
#include <stdio.h>
#include "Log.h"

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
    Semaphore sem2(100, 10);
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
    REQUIRE(false);
  }
  catch(std::runtime_error& ex)
  {
    REQUIRE(std::string(ex.what()) == "Thread exited with exception: exception thread");
  }

  thread.stop();

  try
  {
    thread.start();
    REQUIRE(false);
  }
  catch(std::runtime_error& ex)
  {
    REQUIRE(std::string(ex.what()) == "Thread exited with exception: exception thread");
  }
}

TEST_CASE("thread/stop", "Test stopping threads")
{
  // TODO: implement thread/stop
}
