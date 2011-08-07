#include "Lethe.h"
#include "LetheException.h"
#include "testCommon.h"
#include "catch/catch.hpp"

using namespace lethe;

// This test case may have issues if any files are left over from previous runs, especially if they fail
TEST_CASE("sharedMemory/structor", "Test shared memory construction and destruction")
{
  SharedMemory* shm1 = NULL;
  SharedMemory* shm2 = NULL;

  // Create a unique string to use with shared memory files
  std::stringstream filename;
  filename << getProcessId();

  // Try to open an existing shared memory file that doesn't exist
  REQUIRE_THROWS_AS(shm1 = new SharedMemory(filename.str() + "-test1"), std::bad_syscall);

  // Create a new shared memory
  shm1 = new SharedMemory(1000, filename.str() + "-test2");

  // Make sure it fails if we try to overwrite it
  REQUIRE_THROWS_AS(shm2 = new SharedMemory(500, filename.str() + "-test2"), std::bad_syscall);

  // Open existing shared memory
  shm2 = new SharedMemory(shm1->name());

  // Check parameters
  REQUIRE(shm1->name() == filename.str() + "-test2");
  REQUIRE(shm2->name() == filename.str() + "-test2");
  REQUIRE((uint8_t*)shm1->end() - (uint8_t*)shm1->begin() == 1000);
  REQUIRE((uint8_t*)shm2->end() - (uint8_t*)shm2->begin() == 1000);
  REQUIRE(shm1->size() == 1000);
  REQUIRE(shm2->size() == 1000);

  delete shm1;
  delete shm2;

  // Make sure shared memory files are gone
  REQUIRE_THROWS_AS(shm1 = new SharedMemory(filename.str() + "-test2"), std::bad_syscall);
}

// This doesn't test across separate process or threads, but with the way memory mapping
//  works, it shouldn't be necessary
TEST_CASE("sharedMemory/data", "Test passing data through shared memory")
{
  const uint32_t shmSize = 1000;
  uint8_t i;
  std::stringstream filename;
  filename << getProcessId() << "-test3";

  SharedMemory shm1(shmSize, filename.str());
  SharedMemory shm2(filename.str());

  // Check parameters
  REQUIRE(shm1.name() == filename.str());
  REQUIRE(shm2.name() == filename.str());
  REQUIRE((uint8_t*)shm1.end() - (uint8_t*)shm1.begin() == shmSize);
  REQUIRE((uint8_t*)shm2.end() - (uint8_t*)shm2.begin() == shmSize);
  REQUIRE(shm1.size() == shmSize);
  REQUIRE(shm2.size() == shmSize);

  i = 0;
  for(uint8_t* data = (uint8_t*)shm1.begin(); data != shm1.end(); ++data)
    *data = i++;

  i = 0;
  for(uint8_t* data = (uint8_t*)shm2.begin(); data != shm2.end(); ++data)
    REQUIRE(*data == i++);
}
