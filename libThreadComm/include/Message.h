#ifndef _THREADCOMM_MESSAGE_H
#define _THREADCOMM_MESSAGE_H

#include "stdint.h"

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

    Header* getHeader() { return m_header; };
    Message* getPrev() { return m_prev; };
    Message* getNext() { return m_next; };
    Message* getLastOnStack() { return m_lastOnStack; };
    Message* getNextOnStack() { return reinterpret_cast<Message*>(reinterpret_cast<char*>(this) + m_size); };
    State getState() { return m_state; };
    uint32_t getSize() { return m_size; };

    void* getDataArea() { return &m_data; };
    static Message* getMessage(void* dataArea) { return reinterpret_cast<Message*>(reinterpret_cast<char*>(dataArea) - sizeof(Message) + sizeof(uint32_t)); };

    void setSize(uint32_t size) { m_size = size; };
    void setPrev(Message* prev) { m_prev = prev; };
    void setNext(Message* next) { m_next = next; };
    void setLastOnStack(Message* lastOnStack) { m_lastOnStack = lastOnStack; };
    void setState(State state) { m_state = state; };

    bool overflowCheck();
    Message* split(uint32_t size);
  private:
    uint32_t& getSecondMagic() { return *reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(&m_data) + m_size - sizeof(Message)); };
  };

}

#endif