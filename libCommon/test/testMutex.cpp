#include "catch.hpp"
#include "Abstraction.h"
#include "Exception.h"


/**
 *
 * Test case: structor
 *
 */

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

  delete [] mutexArray;
}


/**
 *
 * Test case: autolock
 *
 */

class MutexTestThread : public Thread
{
public:
  MutexTestThread(Mutex& mutex, Event& event);
  bool isIterating();

private:
  Mutex& m_mutex;
  Event& m_event;
  bool m_iterating;

  void iterate(Handle handle)
  {
    m_iterating = true;

    if(handle != m_mutex.getHandle())
    {
      m_iterating = false;
      throw Exception("Iterate called with invalid parameter");
    }

    if(WaitForObject(m_event.getHandle(), 2000) != WaitSuccess)
    {
      m_iterating = false;
      throw Exception("Thread did not receive secondary event");
    }

    stop();
    m_mutex.unlock();
    m_iterating = false;
  };

  void abandoned(Handle handle __attribute__((unused)))
  {
    throw Exception("Mutex abandoned");
  };

};

MutexTestThread::MutexTestThread(Mutex& mutex, Event& event) :
  Thread(INFINITE),
  m_mutex(mutex),
  m_event(event),
  m_iterating(false)
{
  addWaitObject(m_mutex.getHandle());
}

bool MutexTestThread::isIterating()
{
  return m_iterating;
}

// This requres Thread to work, which requires Event and HandleSet to work
//  If this test case is failing, it could be due to a problem in Thread,
//  Event, or HandleSet.
TEST_CASE("mutex/autolock", "Test auto-lock and unlock with multiple waiting threads")
{
  const uint32_t threadCount = 20;
  MutexTestThread* threadArray[threadCount];
  HandleSet activeThreads;
  Mutex mutex(true);
  Event event(false, true);

  for(uint32_t i = 0; i < threadCount; ++i)
  {
    threadArray[i] = new MutexTestThread(mutex, event);
    activeThreads.add(threadArray[i]->getHandle());
    threadArray[i]->start();
  }

  // Kick off the chain
  mutex.unlock();

  while(activeThreads.getSize() > 0)
  {
    uint32_t iterateCount = 0;
    Handle exitedThread;

    // Wait some time time make sure no threads exit prematurely
    REQUIRE(activeThreads.waitAny(250, exitedThread) == WaitTimeout);

    // Loop through the threads, make sure exactly one is in iterate
    for(uint32_t i = 0; i < threadCount; ++i)
      iterateCount += threadArray[i]->isIterating();

    REQUIRE(iterateCount == 1);

    // Set the event to let the thread exit
    event.set();

    // Check that the thread finishes
    REQUIRE(activeThreads.waitAny(100, exitedThread) == WaitSuccess);
    activeThreads.remove(exitedThread);
  }

  for(uint32_t i = 0; i < threadCount; ++i)
  {
    REQUIRE(threadArray[i]->isStopping() == true);
    REQUIRE(threadArray[i]->getError().length() == 0);
    delete threadArray[i];
  }
}


/**
 *
 * Test case: exception
 *
 */

class ExceptionTestThread : public Thread
{
public:
  ExceptionTestThread(Mutex& mutex);

private:
  Mutex& m_mutex;

  void iterate(Handle handle)
  {
    if(handle != INVALID_HANDLE_VALUE)
      throw Exception("Iterate called with wrong parameter");

    m_mutex.unlock();
    stop();
  };

  void abandoned(Handle handle __attribute__((unused)))
  {
    throw Exception("Mutex abandoned");
  };
};

ExceptionTestThread::ExceptionTestThread(Mutex& mutex) :
  Thread(0), // call iterate as soon as the thread starts
  m_mutex(mutex)
{
  // Do nothing
}

TEST_CASE("mutex/exception", "Test that thread id is enforced")
{
  Mutex mutex(true);
  Event event(false, true);
  ExceptionTestThread thread(mutex);

  {
    thread.start();

    REQUIRE(WaitForObject(thread.getHandle(), 1000) == WaitSuccess);
    REQUIRE(thread.isStopping());
    REQUIRE(thread.getError() == "Cannot unlock mutex from a different thread");
  }
}
