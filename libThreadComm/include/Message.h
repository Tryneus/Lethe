#ifndef _THREADCOMM_MESSAGE_H
#define _THREADCOMM_MESSAGE_H

#include "Abstraction.h"

namespace ThreadComm
{

  class Header;

  class Message
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
    Header* m_header;
    Message* volatile m_prev;
    Message* volatile m_next;
    Message* m_lastOnStack;
    uint32_t m_size;
    State m_state;
    uint32_t m_magic;

    // The data field begins here, but this is used to
    //  account for the magic number at the end of the
    //  data buffer
    uint32_t m_data;

  public:
    Message(Header* header, uint32_t size, State state);

    Header* getHeader();
    Message* getPrev();
    Message* getNext();
    Message* getLastOnStack();
    Message* getNextOnStack();
    State getState();
    uint32_t getSize();

    void* getDataArea();
    static Message* getMessage(void* dataArea);

    void setSize(uint32_t size);
    void setPrev(Message* prev);
    void setNext(Message* next);
    void setLastOnStack(Message* lastOnStack);
    void setState(State state);

    bool overflowCheck();
    Message* split(uint32_t size);

  private:
    uint32_t& getSecondMagic();
  };

}

#endif
