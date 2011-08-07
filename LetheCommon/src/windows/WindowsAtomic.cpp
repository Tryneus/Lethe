#include "windows/WindowsAtomic.h"
#include "Windows.h"
#include <intrin.h>

using namespace lethe;

WindowsAtomic32::WindowsAtomic32(const uint32_t value) :
  m_data(value)
{
  // Do nothing
}

uint32_t WindowsAtomic32::set(uint32_t value)
{
  return m_data = value;
}

uint32_t WindowsAtomic32::add(uint32_t value)
{
  return _InterlockedExchangeAdd(&m_data, value);
}

uint32_t WindowsAtomic32::subtract(uint32_t value)
{
  value = ~value;
  return _InterlockedExchangeAdd(&m_data, value);
}

uint32_t WindowsAtomic32::increment()
{
  return InterlockedIncrement(&m_data);
}

uint32_t WindowsAtomic32::decrement()
{
  return InterlockedDecrement(&m_data);
}

uint32_t WindowsAtomic32::bitwiseAnd(uint32_t value)
{
  return _InterlockedAnd(&m_data, value);
}

uint32_t WindowsAtomic32::bitwiseOr(uint32_t value)
{
  return _InterlockedOr(&m_data, value);
}

uint32_t WindowsAtomic32::bitwiseXor(uint32_t value)
{
  return _InterlockedXor(&m_data, value);
}

bool WindowsAtomic32::bitTestAndSet(uint32_t bit)
{
  return (InterlockedBitTestAndSet(&m_data, bit) != 0);
}

bool WindowsAtomic32::bitTestAndReset(uint32_t bit)
{
  return (InterlockedBitTestAndReset(&m_data, bit) != 0);
}

#if defined(_M_IA64) || defined(_M_X64)

WindowsAtomic64::WindowsAtomic64(const uint64_t value) :
  m_data(value)
{
  // Do nothing
}

uint64_t WindowsAtomic64::set(uint64_t value)
{
  return InterlockedExchange64(&m_data, value);
}

uint64_t WindowsAtomic64::add(uint64_t value)
{
  return _InterlockedExchangeAdd64(&m_data, value);
}

uint64_t WindowsAtomic64::subtract(uint64_t value)
{
  value = ~value;
  return _InterlockedExchangeAdd64(&m_data, value);
}

uint64_t WindowsAtomic64::increment()
{
  return InterlockedIncrement64(&m_data);
}

uint64_t WindowsAtomic64::decrement()
{
  return InterlockedDecrement64(&m_data);
}

uint64_t WindowsAtomic64::bitwiseAnd(uint64_t value)
{
  return _InterlockedAnd64(&m_data, value);
}

uint64_t WindowsAtomic64::bitwiseOr(uint64_t value)
{
  return InterlockedOr64(&m_data, value);
}

uint64_t WindowsAtomic64::bitwiseXor(uint64_t value)
{
  return InterlockedXor64(&m_data, value);
}

bool WindowsAtomic64::bitTestAndSet(uint32_t bit)
{
  return (_bittestandset64(const_cast<LONG64*>(&m_data), bit) != 0);
}

bool WindowsAtomic64::bitTestAndReset(uint32_t bit)
{
  return (_bittestandreset64(const_cast<LONG64*>(&m_data), bit) != 0);
}

#endif
