#ifndef _PROCESSCOMM_H
#define _PROCESSCOMM_H

#if defined(__WIN32__) || defined(_WIN32)

  namespace lethe
  {
    class WindowsProcessByteStream;
    class WindowsProcessMessageStream;
    class WindowsHandleTransfer;

    typedef WindowsProcessByteStream ProcessByteStream;
    typedef WindowsProcessMessageStream ProcessMessageStream;
    typedef WindowsHandleTransfer HandleTransfer;
  }

  #include "ByteStream/WindowsProcessByteStream.h"
  #include "MessageStream/WindowsProcessMessageStream.h"
  #include "WindowsHandleTransfer.h"

#elif defined(__linux__)

  namespace lethe
  {
    class LinuxProcessByteStream;
    class LinuxProcessMessageStream;
    class LinuxHandleTransfer;

    typedef LinuxProcessByteStream ProcessByteStream;
    typedef LinuxProcessMessageStream ProcessMessageStream;
    typedef LinuxHandleTransfer HandleTransfer;
  }

  #include "ByteStream/LinuxProcessByteStream.h"
  #include "MessageStream/LinuxProcessMessageStream.h"
  #include "LinuxHandleTransfer.h"

#else
  #error "Platform not detected"
#endif

#endif
