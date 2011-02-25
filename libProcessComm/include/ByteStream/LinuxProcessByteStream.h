#ifndef _LINUXPROCESSBYTESTREAM_H
#define _LINUXPROCESSBYTESTREAM_H

#include "Abstraction.h"

class LinuxProcessByteStream
{
public:
  LinuxProcessByteStream(uint32_t remoteProcessId);
  ~LinuxProcessByteStream();

  operator WaitObject&();
  Handle getHandle() const;

  void send(void* buffer, uint32_t size);
  uint32_t receive(void* buffer, uint32_t size);

private:
  Handle m_pipeOut;
  Handle m_pipeIn;
};

#endif
