#ifndef _THREADMESSAGE_H
#define _THREADMESSAGE_H

#include "Lethe.h"

namespace lethe
{
  class ThreadMessageHeader;

  class ThreadMessage
  {
  public:

    enum State
    {
      Free,
      Alloc,
      Sent,
      Recv,
      Nil,
      Pend
    };

  private:
    ThreadMessageHeader* m_header;
    ThreadMessage* volatile m_prev;
    ThreadMessage* volatile m_next;
    ThreadMessage* m_lastOnStack;
    uint32_t m_size;
    State m_state;
    uint32_t m_magic;

    // The data field begins here, but this is used to
    //  account for the magic number at the end of the
    //  data buffer
    uint32_t m_data;

  public:
    ThreadMessage(ThreadMessageHeader* header, uint32_t size, State state);

    ThreadMessageHeader* getHeader();
    ThreadMessage* getPrev();
    ThreadMessage* getNext();
    ThreadMessage* getLastOnStack();
    ThreadMessage& getNextOnStack();
    State getState() const;
    uint32_t getSize() const;

    void* getDataArea();
    static ThreadMessage* getMessage(void* dataArea);

    void setSize(uint32_t size);
    void setPrev(ThreadMessage* prev);
    void setNext(ThreadMessage* next);
    void setLastOnStack(ThreadMessage* lastOnStack);
    void setState(State state);

    bool overflowCheck();
    ThreadMessage* split(uint32_t size);

  private:
    uint32_t& getSecondMagic();
  };

}

#endif
