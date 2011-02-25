#ifndef _PROCESSCOMM_H
#define _PROCESSCOMM_H

#if defined(__WIN32__) || defined(_WIN32)

  class WindowsProcessByteStream;

  typedef WindowsProcessByteStream ProcessByteStream;

  #include "ByteStream/WindowsProcessByteStream.h"

#elif defined(__linux__)

  class LinuxProcessByteStream;

  typedef LinuxProcessByteStream ProcessByteStream;

  #include "ByteStream/LinuxProcessByteStream.h"

#else
  #error "Platform not detected"
#endif

#include "MessageStream/ProcessMessageStream.h"

#endif
