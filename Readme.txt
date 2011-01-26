Use Makefiles on Linux (gcc) or Visual Studio for Windows (tested on 2008)

The Linux build requires eventfd with EFD_SEMAPHORE support (should be 2.6.30 or later)


Modules:


libCommon
  libCommon is meant to be a general-purpose library for platform-agnostic multi-threaded applications.  At the moment, it contains Windows and Linux implementations of the following classes:

    Thread
    Mutex
    Semaphore
    Pipe
    Event
    Timer

  Also included are HandleSets (used to aggregate the synchronization objects above to perform multiplexed waits), a Log class (providing a thread-safe ostream-style interface), a base Exception class (nothing special), and a base Singleton class.


libThreadComm
libProcessComm
libSocketComm
  These libraries are meant to abstract both stream and datagram-based communications into a common interface, and the appropriate class may be instatiated based on the relation of the two end-points (libThreadComm for inter-thread, libProcessComm for inter-process, and libSocketComm for inter-machine).
  At the moment, only datagram-based inter-thread is implemented, and a design hasn't been decided on for the final interface.  The rest are rather straightforward, though, using pipes, TCP/IP, or UDP/IP.  Inter-process datagram will use shared memory (much like inter-thread), but the implementation will be slightly different due to limitations of shared memory.


libThreadUtil
  libThreadUtil is just a placeholder at the moment, but the current plan involves two classes: ThreadRegistry and CommRegistry, which are meant to act as central points for creating/looking up threads and IPC.


ThreadCommTest
  The only test application at the moment, this spawns two types of threads (sender and echo), which bounce messages back and forth until the test expires.  This is an alright surface-level test as it contains most of the classes in libCommon, but a more in-depth test suite is needed in the future.


