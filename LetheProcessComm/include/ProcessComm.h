#ifndef _PROCESSCOMM_H
#define _PROCESSCOMM_H

namespace lethe
{
  class ProcessByteStream;
  class ProcessMessageStream;
  class TempProcessStream;
}

#include "ByteStream/ProcessByteStream.h"
#include "MessageStream/ProcessMessageStream.h"
#include "TempProcessStream.h"

#if defined(__WIN32__) || defined(_WIN32)

  namespace lethe
  {
    class WindowsHandleTransfer;
    typedef WindowsHandleTransfer HandleTransfer;
  }

  #include "WindowsHandleTransfer.h"

#elif defined(__linux__)

  namespace lethe
  {
    class LinuxHandleTransfer;
    typedef LinuxHandleTransfer HandleTransfer;
  }

  #include "LinuxHandleTransfer.h"

#else
  #error "Platform not detected"
#endif

#endif
