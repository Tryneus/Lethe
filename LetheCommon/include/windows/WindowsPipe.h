#ifndef _WINDOWSPIPE_H
#define _WINDOWSPIPE_H

#include "LetheTypes.h"
#include "WindowsMutex.h"
#include "WindowsSharedMemory.h"
#include "WindowsEvent.h"
#include "WindowsAtomic.h"
#include <string>

/*
 * The WindowsPipe class encapsulates an implementation of a pipe in Windows.
 *  The Handle of this object is the Handle to an Event indicating that there
 *  is data to read.  The pipe may be one-way using the default constructor, or
 *  two-way if two names are specified.  Other processes on the system may open
 *  the same pipe files if they provide the same names.  An unlimited number of
 *  processes may open the same two-way pipe and use it concurrently.  The
 *  direction of the pipe depends on the order of names provided.  'In' on one
 *  side should correspond to 'out' on the other side.
 *
 * Care should be taken when destroying a pipe so that a partially completed
 *  send is not interrupted.  This may be done by calling flush() before
 *  destruction, which will block until the send completes.  Be careful, as
 *  this may block indefinitely if there is nothing reading data on the other
 *  side.
 *
 * If the pipe buffer is full when a send occurs, or the data is too large to
 *  fit in the buffer, a thread will be spawned to finish the send.  Only one
 *  thread will be used system-wide for this pipe, so all other sends will fail
 *  until the thread completes.
 *
 * To implement the pipe object, a shared memory area of 64k is used as a
 *  circular buffer to contain the data being transmitted.  Two events and two
 *  mutexes are used to notify and control access to data members.  The
 *  writeEvent is used to indicate that data is available, and the readEvent is
 *  used to indicate that a read has been performed.  The writeMutex keeps only
 *  one thread writing data at a time, and the readMutex keeps only one thread
 *  reading data at a time, as well as the creation of any asynchronous send
 *  threads and setting/resetting the readEvent. 
 */
namespace lethe
{
  class WindowsPipe
  {
  public:
    WindowsPipe();

    WindowsPipe(const std::string& pipeIn,
                bool createIn,
                const std::string& pipeOut,
                bool createOut);

    ~WindowsPipe();

    void send(const void* buffer, uint32_t bufferSize);
    uint32_t receive(void* buffer, uint32_t bufferSize);

    bool flush(uint32_t timeout);

    // Act like a wait object without actually being one - passthrough to m_readEventIn
    operator WaitObject&();
    Handle getHandle() const;

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsPipe(const WindowsPipe&);
    WindowsPipe& operator = (const WindowsPipe&);

    struct ShmLayout
    {
      uint32_t pendingWrite;
      uint32_t inOffset;
      uint32_t outOffset;
      unsigned char data[65536];
    };

    struct AsyncData
    {
      WindowsPipe* instance;
      const char* buffer;
      uint32_t size;
    };

    static const std::string generateName();
    static const std::string s_shmNameSuffix;
    static const std::string s_readEventNameSuffix;
    static const std::string s_writeEventNameSuffix;
    static const std::string s_readMutexNameSuffix;
    static const std::string s_writeMutexNameSuffix;
    static WindowsAtomic s_uniqueId;

    static void initializeShm(void* shm);
    void cleanup();
    uint32_t getFreeSpace(ShmLayout* shm);
    uint32_t getUsedSpace(ShmLayout* shm);
    void writeBuffer(const void* buffer, uint32_t size);

    static DWORD WINAPI asyncThreadHook(void* p);
    void asyncThread(const char* buffer, uint32_t size);

    // Variables used when reading
    const std::string m_nameIn;
    WindowsSharedMemory* m_memoryIn;
    WindowsMutex* m_writeMutexIn;
    WindowsMutex* m_readMutexIn;
    WindowsEvent* m_readEventIn;
    WindowsEvent* m_writeEventIn;

    // Variables used when writing
    const std::string m_nameOut;
    WindowsSharedMemory* m_memoryOut;
    WindowsMutex* m_writeMutexOut;
    WindowsMutex* m_readMutexOut;
    WindowsEvent* m_readEventOut;
    WindowsEvent* m_writeEventOut;

    Handle m_asyncThread;
    bool m_destructing;
  };
}

#endif
