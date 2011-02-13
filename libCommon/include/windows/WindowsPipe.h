#ifndef _WINDOWSPIPE_H
#define _WINDOWSPIPE_H

#include "WaitObject.h"
#include "WindowsMutex.h"
#include "WindowsEvent.h"
#include "AbstractionTypes.h"

/*
 * The WindowsPipe class encapsulates an anonymous pipe in Windows.  The
 *  getHandle function returns a Handle to the read side of the pipe.  If a send
 *  fails outright, an exception will be thrown.  For a partly completed send,
 *  the unsent part is buffered and will be pushed through by subsequent send
 *  operations.
 */
class WindowsPipe : public WaitObject
{
public:
  WindowsPipe();
  ~WindowsPipe();

  void send(const void* buffer, uint32_t bufferSize);
  uint32_t receive(void* buffer, uint32_t bufferSize);

private:
  void updateDataEvent(uint32_t bytesWritten);

  WindowsMutex m_mutex;
  WindowsEvent m_dataEvent;
  uint32_t m_dataCount;

  Handle m_pipeRead;
  Handle m_pipeWrite;
  uint8_t* m_pendingData;
  uint8_t* m_pendingSend;
  uint32_t m_pendingSize;
};

#endif
