#ifndef _WINDOWSATOMIC_H
#define _WINDOWSATOMIC_H

#include "Windows.h"
#include "stdint.h"

namespace lethe
{
  class WindowsAtomic32
  {
  public:
    WindowsAtomic32(const uint32_t value);

    uint32_t add(uint32_t value); // return result
    uint32_t bitwiseAnd(uint32_t value); // return original value
    bool bitTestAndSet(uint32_t bit); // return value of bit
    bool bitTestAndReset(uint32_t bit); // return value of bit
    uint32_t compareAndExchange(uint32_t newValue, uint32_t compareValue); // return original value
    uint32_t increment(); // return result
    uint32_t decrement(); // return result
    uint32_t set(uint32_t value); // return original value
    uint32_t bitwiseOr(uint32_t value); // return original value
    uint32_t bitwiseXor(uint32_t value); // return original value

  private:
    volatile LONG m_data;
  };

  #if defined(_M_IA64) || defined(_M_X64)
  // TODO: data alignment issues?
  class WindowsAtomic64
  {
  public:
    WindowsAtomic64(const uint64_t value);

    uint64_t add(uint64_t value); // return result // not available on x86
    uint64_t bitwiseAnd(uint64_t value); // return original value
    bool bitTestAndSet(uint32_t bit); // return value of bit
    bool bitTestAndReset(uint32_t bit); // return value of bit
    uint64_t compareAndExchange(uint64_t value, uint64_t compare); // return original value
    uint64_t increment(); // return result
    uint64_t decrement(); // return result
    uint64_t set(uint64_t value); // return original value
    uint64_t bitwiseOr(uint64_t value); // return original value
    uint64_t bitwiseXor(uint64_t value); // return original value

  private:
    volatile LONG64 m_data;
  };

  typedef WindowsAtomic64 WindowsAtomic;
  #else
  typedef WindowsAtomic32 WindowsAtomic;
  #endif
}

#endif