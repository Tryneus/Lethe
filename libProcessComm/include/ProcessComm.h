#ifndef _PROCESSCOMM_H
#define _PROCESSCOMM_H

#if defined(__WIN32__) || defined(_WIN32)

  class WindowsProcessByteStream;
  class WindowsProcessMessageStream;

  typedef WindowsProcessByteStream ProcessByteStream;
  typedef WindowsProcessMessageStream ProcessMessageStream;

  #include "ByteStream/WindowsProcessByteStream.h"
  #include "MessageStream/WindowsProcessMessageStream.h"

#elif defined(__linux__)

  class LinuxProcessByteStream;
  class LinuxProcessMessageStream;

  typedef LinuxProcessByteStream ProcessByteStream;
  typedef LinuxProcessMessageStream ProcessMessageStream;

  #include "ByteStream/LinuxProcessByteStream.h"
  #include "MessageStream/LinuxProcessMessageStream.h"

#else
  #error "Platform not detected"
#endif

#endif
