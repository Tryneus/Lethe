#ifndef _SOCKETCOMM_H
#define _SOCKETCOMM_H

#if defined(__WIN32__) || defined(_WIN32)

  class WindowsSocketByteStream;
  class WindowsSocketMessageStream;

  typedef WindowsSocketByteStream SocketByteStream;
  typedef WindowsSocketMessageStream SocketMessageStream;

  #include "ByteStream/WindowsSocketByteStream.h"
  #include "MessageStream/WindowsSocketMessageStream.h"

#elif defined(__linux__)

  class LinuxSocketByteStream;
  class LinuxSocketMessageStream;

  typedef LinuxSocketByteStream SocketByteStream;
  typedef LinuxSocketMessageStream SocketMessageStream;

  #include "ByteStream/LinuxSocketByteStream.h"
  #include "MessageStream/LinuxSocketMessageStream.h"

#else
  #error "Platform not detected"
#endif

#endif
