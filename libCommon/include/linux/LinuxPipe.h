#ifndef _LINUXPIPE_H
#define _LINUXPIPE_H

#include "WaitObject.h"
#include "AbstractionTypes.h"
#include <unistd.h>
#include <aio.h>

/*
 * The LinuxPipe class encapsulates an anonymous pipe in Linux.  The getHandle
 *  function returns a file descriptor to the read side of the pipe.  If a send
 *  fails outright, an exception will be thrown.  For a partly completed send,
 *  the unsent part is buffered and will be pushed through by subsequent send
 *  operations.
 */
class LinuxPipe : public WaitObject
{
public:
  LinuxPipe();
  ~LinuxPipe();

  void send(const void* buffer, uint32_t bufferSize);
  uint32_t receive(void* buffer, uint32_t bufferSize);

private:
  static const uint32_t s_maxAsyncEvents = 10;

  void asyncWrite(const void* buffer, uint32_t bufferSize);
  void getAsyncResults();

  Handle m_pipeRead;
  Handle m_pipeWrite;
  uint32_t m_asyncStart;
  uint32_t m_asyncEnd;
  struct aiocb m_asyncArray[s_maxAsyncEvents];
  bool m_blockingWrite;
};

#endif
