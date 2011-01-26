#ifndef _WINDOWSHANDLESET_H
#define _WINDOWSHANDLESET_H

#include "stdint.h"
#include "Windows.h"
#include <set>

#define WaitSuccess    0
#define WaitAbandoned -1
#define WaitTimeout   -2

class WindowsHandleSet
{
public:
  WindowsHandleSet();
  ~WindowsHandleSet();

  void add(HANDLE handle);
  void remove(HANDLE handle);

  uint32_t getSize() const;
  const std::set<HANDLE>& getSet() const;

  int waitAll(uint32_t timeout, HANDLE& handle);
  int waitAny(uint32_t timeout, HANDLE& handle);

private:
  void resizeEvents();

  std::set<HANDLE> m_handleSet;
  HANDLE* m_handleArray;
  HANDLE m_brokenHandle;

  uint32_t m_offset;
};

#endif
