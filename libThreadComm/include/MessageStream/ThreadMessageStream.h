#ifndef _THREADMESSAGESTREAM_H
#define _THREADMESSAGESTREAM_H

#include "Lethe.h"
#include "MessageStream/ThreadMessageHeader.h"

namespace lethe
{
  class ThreadMessageStream : public MessageStream
  {
  public:
    ThreadMessageStream(ThreadMessageHeader& in,
                        ThreadMessageHeader& out,
                        WaitObject& obj);
    ~ThreadMessageStream();

    operator WaitObject&();
    Handle getHandle() const;

    void* allocate(uint32_t size);
    void  send(void* msg);
    void* receive();
    void  release(void* msg);

  private:
    ThreadMessageHeader& m_in;
    ThreadMessageHeader& m_out;
    WaitObject& m_waitObject;
  };
}

#endif
