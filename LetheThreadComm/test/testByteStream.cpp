#include "Lethe.h"
#include "LetheException.h"
#include "ThreadComm.h"
#include "Log.h"
#include "catch/catch.hpp"

using namespace lethe;

TEST_CASE("byteStream/structor", "Test construction and destruction of ThreadByteStream")
{
  ThreadByteConnection byteConnection;
}

TEST_CASE("byteStream/data", "Test passing data through a ThreadByteStream")
{
  ThreadByteConnection byteConnection;
  ByteStream& streamA = byteConnection.getStreamA();
  ByteStream& streamB = byteConnection.getStreamB();

  std::string data("test text");
  char buffer[100];

  streamA.send(data.c_str(), data.length() + 1);

  REQUIRE(WaitForObject(streamB, 0) == WaitSuccess);
  REQUIRE(WaitForObject(streamA, 20) == WaitTimeout);
  REQUIRE(streamB.receive(buffer, 100) == data.length() + 1);
  REQUIRE(data == buffer);

  REQUIRE(WaitForObject(streamA, 20) == WaitTimeout);
  REQUIRE(WaitForObject(streamB, 20) == WaitTimeout);

  data.assign("text test");
  streamB.send(data.c_str(), data.length() + 1);

  REQUIRE(WaitForObject(streamA, 0) == WaitSuccess);
  REQUIRE(WaitForObject(streamB, 20) == WaitTimeout);
  REQUIRE(streamA.receive(buffer, 100) == data.length() + 1);
  REQUIRE(data == buffer);

  REQUIRE(WaitForObject(streamA, 20) == WaitTimeout);
  REQUIRE(WaitForObject(streamB, 20) == WaitTimeout);
}

TEST_CASE("byteStream/largeData", "Test passing excessively large data through a ThreadByteStream")
{
  // TODO: implement largeData test
}
