#ifndef _SOCKETCOMM_H
#define _SOCKETCOMM_H

#if defined(__WIN32__) || defined(_WIN32)

  namespace lethe
  {
    class WindowsSocketByteStream;
    class WindowsSocketMessageStream;
    class WindowsSocketByteStreamListener;
    class WindowsSocketMessageStreamListener;

    typedef WindowsSocketByteStream SocketByteStream;
    typedef WindowsSocketMessageStream SocketMessageStream;
    typedef WindowsSocketByteStreamListener SocketByteStreamListener;
    typedef WindowsSocketMessageStreamListener SocketMessageStreamListener;
  }

  #include "ByteStream/WindowsSocketByteStream.h"
  #include "MessageStream/WindowsSocketMessageStream.h"
  #include "WindowsSocketListener.h"

#elif defined(__linux__)

  namespace lethe
  {
    class LinuxSocketByteStream;
    class LinuxSocketMessageStream;
    class LinuxSocketByteStreamListener;
    class LinuxSocketMessageStreamListener;

    typedef LinuxSocketByteStream SocketByteStream;
    typedef LinuxSocketMessageStream SocketMessageStream;
    typedef LinuxSocketByteStreamListener SocketByteStreamListener;
    typedef LinuxSocketMessageStreamListener SocketMessageStreamListener;
  }

  #include "ByteStream/LinuxSocketByteStream.h"
  #include "MessageStream/LinuxSocketMessageStream.h"
  #include "LinuxSocketListener.h"

#else
  #error "Platform not detected"
#endif

#endif
