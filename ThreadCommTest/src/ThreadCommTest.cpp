#include "ThreadComm.h"
#include "SenderThread.h"
#include "EchoThread.h"
#include "Abstraction.h"
#include "Exception.h"
#include "Log.h"

int main()
{
  // Seed RNG
  {
    uint32_t randomSeed = seedRandom(0);
    LogInfo("Test seeded with " << randomSeed);
  }

  try
  {
    ThreadComm::Connection conn(500000, 500000);

    SenderThread sender(conn.getChannelA());
    EchoThread echo(conn.getChannelB());

    echo.start();
    sender.start();

    // Let the threads run for 10 seconds, then stop them
    Sleep(100000);

    sender.stop();
    echo.stop();

    // Wait for threads to exit
    HandleSet handles;
    Handle finished;

    handles.add(sender.getHandle());
    handles.add(echo.getHandle());

    // TODO: this can be simplified if waitAll is implemented for linux    
    // if(handles.waitAll(2000, finished) != WaitSuccess)
    //   throw Exception("Threads did not stop in a timely fashion");
    LogInfo("Waiting for threads to exit");
    while(handles.getSize() != 0)
    {
      if(handles.waitAny(2000, finished) != WaitSuccess)
        throw Exception("Threads did not stop in a timely fashion");

      handles.remove(finished);
      LogInfo("Thread " << (uint32_t)finished << " exited");
    }
  }
  catch(Exception& ex)
  {
    LogError("Exception encountered, " << ex.what());
  }

  return 0;
}
