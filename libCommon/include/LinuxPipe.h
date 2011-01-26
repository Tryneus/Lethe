#ifndef _LINUXPIPE_H
#define _LINUXPIPE_H

#include <stdint.h>
#include <unistd.h>

class LinuxPipe
{
public:
  LinuxPipe();
  ~LinuxPipe();

  void send(uint8_t* buffer, uint32_t bufferSize);
  uint32_t receive(uint8_t* buffer, uint32_t bufferSize);

  int getHandle();

private:
  int m_pipeRead;
  int m_pipeWrite;
  uint8_t* m_pendingData;
  uint8_t* m_pendingSend;
  ssize_t m_pendingSize;
};

#endif
