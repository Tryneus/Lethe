#ifndef _WINDOWSHANDLESET_H
#define _WINDOWSHANDLESET_H

#include "stdint.h"
#include "Windows.h"
#include <set>

#define WaitSuccess    0
#define WaitAbandoned -1
#define WaitTimeout   -2

/*
 * The WindowsHandleSet class provides a method of grouping and waiting on multiple
 *  handles.  Handles may be added and removed, and when a wait function is called,
 *  it will return the result of the wait as well as the handle that triggered the
 *  wakeup.  If there is an error on a handle, the wait result will be WaitAbandoned,
 *  and the user should take measures to fix or remove the broken handle.
 *
 * The underlying system call used for waiting is WaitForMultipleObjects, which
 *  has a built-in unfairness when used with the same set of Handles under high load.
 *  Because of this, handles occurring earlier in the set will be favored.  In order
 *  to combat this, the handle array is doubled and the pointer provided to
 *  WaitForMultipleObjects is moved around.  After receiving an event on a handle,
 *  the pointer is moved to just after that handle in the array, so it will be the
 *  least favored in the next call.
 *
 * Note that while waitAll is implemented on Windows, it is not on Linux, and there
 *  are no plans at the moment on how to implement it there.  For the most portability,
 *  only use waitAny.
 */
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
