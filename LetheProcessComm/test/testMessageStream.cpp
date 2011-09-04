#include "Lethe.h"
#include "LetheInternal.h"
#include "ProcessComm.h"
#include "ThreadComm.h"
#include "catch/catch.hpp"

using namespace lethe;

TEST_CASE("messageStream/structor", "Test construction and destruction of a ProcessMessageStream")
{
  std::vector<std::string> args;
  args.push_back("--echo");
  args.push_back("--message-stream");

  // Use ThreadByteStreams since we're in the same process
  for(uint32_t i = 0; i < 1; ++i)
  {
    uint32_t childPid = lethe::createProcess("../bin/testProcess", args);
    ProcessMessageStream stream(childPid, 65536, 2000); // Allow 2 seconds to connect
  }
}
