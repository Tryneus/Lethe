#ifndef _LINUXPIPE_H
#define _LINUXPIPE_H

#include "LinuxAtomic.h"
#include "WaitObject.h"
#include "ByteStream.h"
#include "LetheTypes.h"
#include <unistd.h>
#include <aio.h>

/*
 * The LinuxPipe class encapsulates an anonymous pipe in Linux.  The Handle
 *  of this object is a file descriptor to the read side of the pipe.  If a send
 *  fails outright, an exception will be thrown.  For a partly completed send,
 *  the unsent part is buffered and will be pushed through by subsequent send
 *  operations.
 */
namespace lethe
{
  // Prototype for transferring handles between processes - defined in libProcessComm
  class LinuxHandleTransfer;

  class LinuxPipe : public ByteStream
  {
  public:
    LinuxPipe();
    LinuxPipe(const std::string& pipeIn, bool createIn, const std::string& pipeOut, bool createOut); // Constructor for a named pipe
    ~LinuxPipe();

    bool flush(uint32_t timeout = INFINITE);
    void send(const void* buffer, uint32_t bufferSize);
    uint32_t receive(void* buffer, uint32_t bufferSize);

    const std::string& getNameIn() const;
    const std::string& getNameOut() const;

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxPipe(const LinuxPipe&);
    LinuxPipe& operator = (const LinuxPipe&);

    // Allow LinuxPipe to be constructed by a handle transfer from another process
    friend class LinuxHandleTransfer;
    LinuxPipe(Handle pipeRead, Handle pipeWrite);

    static const std::string s_fifoPath;
    static const std::string s_fifoBaseName;
    static LinuxAtomic s_uniqueId;

    void setupAsync();
    void cleanup();

    Handle m_pipeRead;
    Handle m_pipeWrite;

    std::string m_fifoReadName;
    std::string m_fifoWriteName;

    bool m_inCreated;
    bool m_outCreated;

    // Since we can't make kernel or libc-based aio work reliably with a full pipe, do it ourselves
    struct AsyncData
    {
      pthread_attr_t attr;
      pthread_t thread;
      uint8_t* buffer;
      uint32_t size;
      uint32_t offset;
      int result;
    };

    void startAsync(uint8_t* buffer, uint32_t size);
    static void* asyncThreadHook(void* param);
    void asyncThreadInternal();

    AsyncData m_async;
  };
}

#endif
