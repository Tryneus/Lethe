  Lethe is a cross-platform synchronization and communication library.  There are 5 sub-libraries providing discrete types of functionality.  LetheCommon provides synchronization as well as other classes used throughout Lethe.  LetheThreadComm, LetheProcessComm, and LetheSocketComm provide communication between threads, processes, and computers, respectively.  LetheThreadUtil adds some extra functionality on top of these to make setting up and using Threads and IPC slightly easier.

LetheCommon
  LetheCommon is meant to be a general-purpose library for platform-agnostic multi-threaded applications.  At the moment, it contains Windows and Linux implementations of the following classes:

  WaitObjects:
    Thread - a base thread class that may be configured to trigger on multiple wait objects
    Mutex - a mutex that automatically locks on a successful wait and enforces thread ownership
    Semaphore - a simple semaphore
    Pipe - a one or two-directional pipe
    Event - a triggerable event that may be set to automatically reset on a successful wait
    Timer - a waitable timer that may be given a timeout in milliseconds
  SharedMemory - a block of memory that may be shared between separate processes
  WaitSet - aggregates WaitObjects above to wait for multiple objects at one time
  Log - provides a thread-safe ostream-style interface
  Singleton - a base singleton class, used by the Log class


LetheThreadComm
LetheProcessComm
LetheSocketComm
  These libraries are meant to abstract various stream and datagram-based communications into common interfaces, and the appropriate class may be instatiated based on the relation of the two end-points (LetheThreadComm for inter-thread, LetheProcessComm for inter-process, and LetheSocketComm for inter-machine). Provided classes:

  ThreadByteStream
  ThreadMessageStream
  ProcessByteStream - implemented, pending testing
  ProcessMessageStream - implemented, pending testing
  SocketByteStream - not yet implemented
  SocketMessageStream - not yet implemented


LetheThreadUtil
  LetheThreadUtil provides a way of tracking Thread objects as well as providing a common interface for creating ByteStreams and MessageStreams to remote processes and computers.  The provided classes are:

  ThreadRegistry - a centralized registry of Thread objects, may provide some extra functionality in the future
  CommRegistry - a centralized registry of all Stream objects in the process, providing a simple interface to create connections with other processes running with an instance of this class
  HandleTransfer - a helper class allowing libCommon synchronization objects to be transferred or shared between processes


Use Makefiles on Linux (gcc) or Visual Studio for Windows (tested on 2008)

Requirements:
  Linux:
    Requires the eventfd kernel module to be loaded (libCommon/module)
    'make valTest' requires 'valgrind' to be installed
    'make check' and 'make checkTest' require 'cppcheck' to be installed
    Almost all synchronization objects use file descriptors.  For large numbers of these, 'ulimit -n' may need to be raised

  Windows:
    Requires some manual modification of thirdParty modules

  Visual Studio:
    Requires TR1 headers to be installed (feature pack/service pack)
    Requires WinRAR to be installed at "C:\Program Files\WinRAR" for the thirdParty.proj pre-build actions.  If not, do the extract/copy manually

  GCC:
    Requires -std=c++0x (for std::atomic)


