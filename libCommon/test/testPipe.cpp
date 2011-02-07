#include "Abstraction.h"
#include "Exception.h"
#include "catch.hpp"

TEST_CASE("pipe/structor", "Test construction/destruction")
{
  // No parameters, just create a lot of pipes and see if we crash
  const uint32_t numPipes = 101;
  uint8_t buffer[100] = { "zffOPqMJONVQqDe2QNBdKWsrZH3rfbxx5cQqCuQkdFe8xnEqj4GvvQgvuGvNTgDt49WZ59zKOg9Ln3TPDJRRkWC1oF74F7MdggB" };
  Pipe pipes[numPipes];

  // Write some data to the pipes to make sure they can still destruct
  for(uint32_t i(0); i < numPipes; ++i)
  {
    for(uint32_t j(0); j < i; ++j)
      pipes[i].send(buffer, 100);
  }

  // Write data to the last pipe until a write fails
  try
  {
    while(true)
      pipes[numPipes - 1].send(buffer, 100);
  }
  catch(OutOfMemoryException&) { }

  // No checks, as long as we don't crash, it's fine
}

TEST_CASE("pipe/send", "Test sending on a pipe")
{
  // TODO: implement pipe/send
}

TEST_CASE("pipe/receive", "Test receiving on a pipe")
{
  // TODO: implement pipe/receive
}

TEST_CASE("pipe/exceptions", "Test exception handling")
{
  // TODO: implement pipe/exceptions
}
