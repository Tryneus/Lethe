#include "Lethe.h"
#include "LetheException.h"
#include "catch.hpp"

using namespace lethe;

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
  catch(std::bad_alloc&) { }

  // No checks, as long as we don't crash, it's fine
}


// Child thread to receive on the pipe
class PipeTestThread : public Thread
{
public:
  PipeTestThread(uint32_t maxData, Pipe& pipe, Event& event);
  ~PipeTestThread();

  void lock();
  void unlock();
  uint32_t getData(const char*& buffer) const;

protected:
  void abandoned(Handle handle);
  void iterate(Handle handle);

private:
  Mutex m_mutex;
  Event& m_event;
  Pipe& m_pipe;
  uint32_t m_maxData;
  char* m_dataBuffer;
  uint32_t m_dataCount;
};

PipeTestThread::PipeTestThread(uint32_t maxData,
                               Pipe& pipe,
                               Event& event) :
  Thread(INFINITE),
  m_mutex(true),
  m_event(event),
  m_pipe(pipe),
  m_maxData(maxData),
  m_dataBuffer(new char[m_maxData]),
  m_dataCount(0)
{
  addWaitObject(m_pipe);
  m_mutex.unlock();
}

PipeTestThread::~PipeTestThread()
{
  delete [] m_dataBuffer;
}

void PipeTestThread::lock()
{
  m_mutex.lock();
}

void PipeTestThread::unlock()
{
  m_mutex.unlock();
}

uint32_t PipeTestThread::getData(const char*& buffer) const
{
  buffer = m_dataBuffer;
  return m_dataCount;
}

void PipeTestThread::iterate(Handle handle)
{
  if(handle == m_pipe.getHandle())
  {
    lock();
    m_dataCount += m_pipe.receive(m_dataBuffer + m_dataCount, m_maxData - m_dataCount);
    unlock();

    if(m_dataCount > m_maxData)
      throw std::logic_error("buffer overflow");

    m_event.set();
  }
  else
    throw std::invalid_argument("invalid handle");
}

void PipeTestThread::abandoned(Handle handle GCC_UNUSED)
{
  throw std::logic_error("Abandoned handle in pipe test thread");
}

TEST_CASE("pipe/data", "Test sending/receiving on a pipe")
{
  const uint32_t bufferSize = 1000;
  std::string data("abdefghijklmnopqrstuvwxyz0123456789");
  const char* threadBuffer;
  Pipe pipe;
  Event event(false, true); // Event for thread to notify once it's received data
  PipeTestThread thread(bufferSize, pipe, event);

  thread.start();

  for(uint32_t i = data.length() + 1; i < bufferSize; i += data.length() + 1)
  {
    pipe.send(data.c_str(), data.length() + 1);

    // Wait for the thread to receive it
    REQUIRE(WaitForObject(event, 100) == WaitSuccess);

    // Verify the entire data buffer each time
    thread.lock();

    REQUIRE(thread.getData(threadBuffer) == i);
    for(uint32_t j = 0; j < i; j += data.length() + 1)
      REQUIRE(data == threadBuffer + j);

    thread.unlock();
  }

  thread.stop();
  REQUIRE(WaitForObject(thread, 100) == WaitSuccess);
  REQUIRE(thread.getError() == "");
}

TEST_CASE("pipe/largedata", "Test sending data buffers too large to fit in the pipe")
{
  const uint32_t bufferCount = 1024 * 12;
  const uint32_t bufferSize = bufferCount * sizeof(int32_t); // 48k buffer
  int32_t* dataBuffer = new int32_t[bufferCount];
  Pipe pipe;
  Event event(false, true);
  PipeTestThread thread(bufferSize * 100, pipe, event); // Reserve way more space than needed

  // Initialize buffer
  for(uint32_t i = 0; i < bufferCount; ++i)
    dataBuffer[i] = rand();

  // This assumes that the pipe buffer is 64k - hardcoded in linux kernel after 2.6.11
  REQUIRE_NOTHROW(pipe.send(dataBuffer, bufferSize)); // First write should complete normally

  // Next 10 should result in asynchronous sends
  REQUIRE_NOTHROW(pipe.send(dataBuffer, bufferSize));
  REQUIRE_NOTHROW(pipe.send(dataBuffer, bufferSize));
  REQUIRE_NOTHROW(pipe.send(dataBuffer, bufferSize));
  REQUIRE_NOTHROW(pipe.send(dataBuffer, bufferSize));
  REQUIRE_NOTHROW(pipe.send(dataBuffer, bufferSize));
  REQUIRE_NOTHROW(pipe.send(dataBuffer, bufferSize));
  REQUIRE_NOTHROW(pipe.send(dataBuffer, bufferSize));
  REQUIRE_NOTHROW(pipe.send(dataBuffer, bufferSize));
  REQUIRE_NOTHROW(pipe.send(dataBuffer, bufferSize));

  // After 10 async sends, an out of memory exception should happen
  REQUIRE_THROWS_AS(pipe.send(dataBuffer, bufferSize), std::bad_alloc);

  thread.start();

  // Keep waiting until the other side is done receiving
  while(WaitForObject(event, 100) == WaitSuccess);

  // Verify the received data
  thread.lock();

  const char* remoteBuffer;
  REQUIRE(thread.getData(remoteBuffer) == 10 * bufferSize);
  const int32_t* remoteBufferInt = reinterpret_cast<const int32_t*>(remoteBuffer);

  bool success = true;
  for(uint32_t i = 0; i < bufferSize * 10; ++i)
    success &= (dataBuffer[i % bufferCount] == remoteBufferInt[i % bufferCount]);

  REQUIRE(success);
  thread.unlock();

  thread.stop();
  REQUIRE(WaitForObject(thread, 100) == WaitSuccess);
  REQUIRE(thread.getError() == "");
  delete [] dataBuffer;
}

TEST_CASE("pipe/exception", "Test pipe error conditions")
{
  // Not many exceptions that can be triggered reliably
  // Try to read from a pipe with no data
  Pipe pipe;
  uint8_t buffer[1];

  REQUIRE_THROWS_AS(pipe.receive(buffer, 1), std::bad_syscall);
}
