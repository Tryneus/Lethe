#include "linux/LinuxAtomic.h"
#include "LetheInternal.h"

using namespace lethe;

LinuxAtomic32::LinuxAtomic32(const uint32_t value) :
  m_data(value)
{
  // Do nothing
}

uint32_t LinuxAtomic32::set(uint32_t value) // return original value
{
  return value;
}

uint32_t LinuxAtomic32::add(uint32_t value) // return result
{
  return (m_data += value);
}

uint32_t LinuxAtomic32::subtract(uint32_t value) // return result
{
  return (m_data -= value);
}

uint32_t LinuxAtomic32::increment() // return result
{
  return ++m_data;
}

uint32_t LinuxAtomic32::decrement() // return result
{
  return --m_data;
}

uint32_t LinuxAtomic32::bitwiseAnd(uint32_t value) // return original value
{
  uint32_t original = m_data;
  m_data &= value;
  return original;
}

uint32_t LinuxAtomic32::bitwiseOr(uint32_t value) // return original value
{
  uint32_t original = m_data;
  m_data |= value;
  return original;
}

uint32_t LinuxAtomic32::bitwiseXor(uint32_t value) // return original value
{
  uint32_t original = m_data;
  m_data ^= value;
  return original;
}

bool LinuxAtomic32::bitTestAndSet(uint32_t bit) // return original value of bit
{
  uint32_t mask = 1 << bit;
  return ((bitwiseOr(mask) & (mask)) > 0);
}

bool LinuxAtomic32::bitTestAndReset(uint32_t bit) // return original value of bit
{
  uint32_t mask = 1 << bit;
  return ((bitwiseAnd(~mask) & (mask)) > 0);
}

#if defined(__x86_64)

LinuxAtomic64::LinuxAtomic64(const uint64_t value) :
  m_data(value)
{
  // Do nothing
}

uint64_t LinuxAtomic64::set(uint64_t value) // return original value
{
  return m_data.exchange(value);
}

uint64_t LinuxAtomic64::add(uint64_t value) // return result
{
  return (m_data += value);
}

uint64_t LinuxAtomic64::subtract(uint64_t value) // return result
{
  return (m_data -= value);
}

uint64_t LinuxAtomic64::increment() // return result
{
  return ++m_data;
}

uint64_t LinuxAtomic64::decrement() // return result
{
  return --m_data;
}

uint64_t LinuxAtomic64::bitwiseAnd(uint64_t value) // return original value
{
  uint64_t original = m_data;
  m_data &= value;
  return original;
}

uint64_t LinuxAtomic64::bitwiseOr(uint64_t value) // return original value
{
  uint64_t original = m_data;
  m_data &= value;
  return original;
}

uint64_t LinuxAtomic64::bitwiseXor(uint64_t value) // return original value
{
  uint64_t original = m_data;
  m_data &= value;
  return original;
}

bool LinuxAtomic64::bitTestAndSet(uint32_t bit) // return original value of bit
{
  uint64_t mask = 1 << bit;
  return ((bitwiseOr(mask) & (mask)) > 0);
}

bool LinuxAtomic64::bitTestAndReset(uint32_t bit) // return original value of bit
{
  uint64_t mask = 1 << bit;
  return ((bitwiseAnd(~mask) & (mask)) > 0);
}

#endif
