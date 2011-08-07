#ifndef _PROCESSMESSAGE_H
#define _PROCESSMESSAGE_H

#include "Lethe.h"

namespace lethe
{
  class ProcessMessageHeader;

  class ProcessMessage
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
    // Private, undefined copy constructor and assignment operator so they can't be used
    ProcessMessage(const ProcessMessage&);
    ProcessMessage& operator = (const ProcessMessage&);

    ProcessMessage* getMessage(uint32_t offset);
    uint32_t getEnd();

    uint32_t& getSecondMagic();

    uint32_t m_offset;
    uint32_t volatile m_prev;
    uint32_t volatile m_next;
    uint32_t m_lastOnStack;
    uint32_t m_size;
    State m_state;
    uint32_t m_magic;
    uint32_t m_data;

  public:
    ProcessMessage(uint32_t offset, uint32_t size, State state);

    uint32_t getOffset() const;
    uint32_t getPrev() const;
    uint32_t getNext() const;
    uint32_t getLastOnStack() const;
    uint32_t getNextOnStack() const;
    uint32_t getSize() const;
    State getState() const;

    void* getDataArea();
    static ProcessMessage* getMessage(void* dataArea);

    void setSize(uint32_t size);
    void setPrev(uint32_t offset);
    void setNext(uint32_t offset);
    void setLastOnStack(uint32_t offset);
    void setState(State state);

    bool overflowCheck();
    ProcessMessage* split(uint32_t size);
  };

}

#endif
