#ifndef _WINDOWSPIPE_H
#define _WINDOWSPIPE_H

#include "Windows.h"
#include "stdint.h"

class WindowsPipe
{
public:
  WindowsPipe();
  ~WindowsPipe();

  void send(uint8_t* buffer, uint32_t bufferSize);
  uint32_t receive(uint8_t* buffer, uint32_t bufferSize);

  HANDLE getHandle();

private:
  HANDLE m_pipeRead;
  HANDLE m_pipeWrite;
  uint8_t* m_pendingData;
  uint8_t* m_pendingSend;
  uint32_t m_pendingSize;
};

#endif
