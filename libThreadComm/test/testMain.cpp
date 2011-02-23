#include "ThreadComm.h"
#include "SenderThread.h"
#include "EchoThread.h"
#include "Abstraction.h"
#include "Log.h"
#include <iostream>

int main()
{
  uint32_t testSeconds = 10;
  SenderThread* sender(NULL);
  EchoThread* echo(NULL);
  ThreadComm::Connection* conn(NULL);

  // Seed RNG
  {
    uint32_t randomSeed = seedRandom(0);
    LogInfo("Test seeded with " << randomSeed);
  }

  try
  {
    conn = new ThreadComm::Connection(500000, 500000);

    // Dynamically allocated so we can destroy them in order
    sender = new SenderThread(conn->getChannelA());
    echo = new EchoThread(conn->getChannelB());

    echo->start();
    sender->start();

    // Let the threads run for some seconds, then stop them
    Sleep(testSeconds * 1000);

    sender->stop();
    echo->stop();

    // Wait for threads to exit
    WaitSet waitSet;
    Handle finished;

    waitSet.add(*sender);
    waitSet.add(*echo);

    // TODO: this can be simplified if waitAll is implemented for linux
    // if(waitSet.waitAll(2000, finished) != WaitSuccess)
    //   throw std::runtime_error("threads did not stop");
    LogInfo("Waiting for threads to exit");
    while(waitSet.getSize() != 0)
    {
      if(waitSet.waitAny(2000, finished) != WaitSuccess)
        throw std::runtime_error("threads did not stop");

      waitSet.remove(finished);
      LogInfo("Thread " << (uint32_t)finished << " exited");
    }

    // Delete in order to push through messages on the line
    delete echo;
    delete sender;
    delete conn;
  }
  catch(std::exception& ex)
  {
    LogError("Exception encountered during test: " << ex.what());

    try
    {
      delete echo;
      delete sender;
      delete conn;
    }
    catch(std::exception& ex)
    {
      LogError("Another exception encountered while destroying threads: " << ex.what());
    }
  }

  return 0;
}
