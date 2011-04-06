#include "ProcessComm.h"
#include "catch.hpp"

using namespace lethe;

TEST_CASE("byteStream/structor", "Test construction and destruction of ProcessByteStream")
{
  ProcessByteStream byteStream(999999, 1000);
}
