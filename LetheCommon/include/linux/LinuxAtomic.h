#ifndef _LINUXATOMIC_H
#define _LINUXATOMIC_H

#include <cstdatomic>
#include <stdint.h>

namespace lethe
{
  class LinuxAtomic32
  {
  public:
    explicit LinuxAtomic32(const uint32_t value);

    uint32_t set(uint32_t value); // return original value
    uint32_t add(uint32_t value); // return result
    uint32_t subtract(uint32_t value); // return result
    uint32_t increment(); // return result
    uint32_t decrement(); // return result
    uint32_t bitwiseAnd(uint32_t value); // return original value
    uint32_t bitwiseOr(uint32_t value); // return original value
    uint32_t bitwiseXor(uint32_t value); // return original value
    bool bitTestAndSet(uint32_t bit); // return original value of bit
    bool bitTestAndReset(uint32_t bit); // return original value of bit

  private:
    std::atomic<uint32_t> m_data;;
  };

  #if defined(__x86_64)

  class LinuxAtomic64
  {
  public:
    explicit LinuxAtomic64(const uint64_t value);

    uint64_t set(uint64_t value); // return original value
    uint64_t add(uint64_t value); // return result
    uint64_t subtract(uint64_t value); // return result
    uint64_t increment(); // return result
    uint64_t decrement(); // return result
    uint64_t bitwiseAnd(uint64_t value); // return original value
    uint64_t bitwiseOr(uint64_t value); // return original value
    uint64_t bitwiseXor(uint64_t value); // return original value
    bool bitTestAndSet(uint32_t bit); // return original value of bit
    bool bitTestAndReset(uint32_t bit); // return original value of bit

  private:
    std::atomic<uint64_t> m_data;;
  };

  typedef LinuxAtomic64 LinuxAtomic;
  #else
  typedef LinuxAtomic32 LinuxAtomic;
  #endif
}

#endif
