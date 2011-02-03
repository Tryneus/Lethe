#include "linux/LinuxThread.h"
#include "Exception.h"

LinuxThread::LinuxThread(uint32_t timeout) :
  BaseThread(timeout)
{
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&m_thread, &attr, threadHook, this);
  pthread_attr_destroy(&attr);
}

LinuxThread::~LinuxThread()
{
  // Do nothing
}

void* LinuxThread::threadHook(void* param)
{
  return reinterpret_cast<LinuxThread*>(param)->threadMain();
}
