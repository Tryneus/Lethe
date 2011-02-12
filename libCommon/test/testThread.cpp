#include "Abstraction.h"
#include "AbstractionException.h"
#include "catch.hpp"
#include "testCommon.h"

// Dummy thread loops until manually stop()ed
class DummyThread : public Thread
{
public:
  DummyThread() : Thread(0) { };
  ~DummyThread() { };

protected:
  void iterate(Handle handle __attribute__((unused))) { /* Do nothing */ };
  void abandoned(Handle handle __attribute__((unused))) { throw std::logic_error("DummyThread received abandoned event"); };
};

// StopThread stops itself after the given timeout
class StopThread : public Thread
{
public:
  StopThread(uint32_t timeout) : Thread(timeout) { };
  ~StopThread() { };

protected:
  void iterate(Handle handle __attribute__((unused))) { stop(); };
  void abandoned(Handle handle __attribute__((unused))) { throw std::logic_error("StopThread received abandoned event"); };
};

// WaitThread waits until the given object triggers, then stops
class WaitThread : public Thread
{
public:
  WaitThread(WaitObject& obj) : Thread(INFINITE) { addWaitObject(obj); };
  ~WaitThread() { }; // TODO: remove wait object?

protected:
  void iterate(Handle handle __attribute__((unused))) { stop(); };
  void abandoned(Handle handle __attribute__((unused))) { throw std::logic_error("WaitThread received abandoned event"); };
};

TEST_CASE("thread/structor", "Test construction/destruction")
{
  uint32_t startTime;

  for(uint32_t i = 0; i < 256; ++i)
  {
    // Create one of each type of thread
    //DummyThread dThread;
    Thread* dThread = new DummyThread();
    Thread* sThread = new StopThread(10);
    Thread* wThread = new WaitThread(*dThread);

//    if(i & bit1)
      dThread->start();
//    if(i & bit2)
      sThread->start();
//    if(i & bit3)
      wThread->start();

//    if(i & bit4)
//      dThread->stop();
//    if(i & bit5)
//      sThread->stop();
//    if(i & bit6)
//      wThread->stop();

    // Make sure the threads destruct successfully in a timely fashion
    startTime = getTime();
    delete sThread;
    delete dThread;
    delete wThread;
    // This is set high because the first run is rather slow with valgrind
    REQUIRE(getTime() - startTime < 350);
  }
}

TEST_CASE("thread/run", "Test running threads")
{
  // TODO: implement thread/run
}

TEST_CASE("thread/exception", "Test thread exception handling")
{
  // TODO: implement thread/exception
}

TEST_CASE("thread/pause", "Test pausing threads")
{
  // TODO: implement thread/pause
}

TEST_CASE("thread/stop", "Test stopping threads")
{
  // TODO: implement thread/stop
}
