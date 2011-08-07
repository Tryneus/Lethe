#include "linux/LinuxAtomic.h"
#include "LetheInternal.h"

using namespace lethe;

LinuxAtomic32::LinuxAtomic32(const uint32_t value) :
  m_data(value)
{
  // Do nothing
}

uint32_t LinuxAtomic32::add(uint32_t value) // return result
{
  return (m_data += value);
}

uint32_t LinuxAtomic32::bitwiseAnd(uint32_t value) // return original value
{
  uint32_t original = m_data;
  m_data &= value;
  return original;
}

bool LinuxAtomic32::bitTestAndSet(uint32_t bit GCC_UNUSED) // return value of bit
{
  // TODO: implement
  return false;
}

bool LinuxAtomic32::bitTestAndReset(uint32_t bit GCC_UNUSED) // return value of bit
{
  // TODO: implement
  return false;
}

uint32_t LinuxAtomic32::compareAndExchange(uint32_t newValue GCC_UNUSED, uint32_t compareValue GCC_UNUSED) // return original value
{
  // TODO: implement
  return 0;
}

uint32_t LinuxAtomic32::increment() // return result
{
  return ++m_data;
}

uint32_t LinuxAtomic32::decrement() // return result
{
  return --m_data;
}

uint32_t LinuxAtomic32::set(uint32_t value) // return original value
{
  return value;
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

#if defined(_M_IA64) || defined(_M_X64)

LinuxAtomic64::LinuxAtomic64(const uint64_t value) :
  m_data(value)
{
  // Do nothing
}

uint64_t LinuxAtomic64::add(uint64_t value) // return result // not available on x86
{
   return (m_data += value);
}

uint64_t LinuxAtomic64::bitwiseAnd(uint64_t value) // return original value
{
  uint64_t original = m_data;
  m_data &= value;
  return original;
}

bool LinuxAtomic64::bitTestAndSet(uint32_t bit GCC_UNUSED) // return value of bit
{
  // TODO: implement
  return false;
}

bool LinuxAtomic64::bitTestAndReset(uint32_t bit GCC_UNUSED) // return value of bit
{
  // TODO: implement
  return false;
}

uint64_t LinuxAtomic64::compareAndExchange(uint64_t newValue GCC_UNUSED, uint64_t compare GCC_UNUSED) // return original value
{
  // TODO: implement
  return 0;
}

uint64_t LinuxAtomic64::increment() // return result
{
  return ++m_data;
}

uint64_t LinuxAtomic64::decrement() // return result
{
  return --m_data;
}

uint64_t LinuxAtomic64::set(uint64_t value) // return original value
{
  return m_data.exchange(value);
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

#endif
