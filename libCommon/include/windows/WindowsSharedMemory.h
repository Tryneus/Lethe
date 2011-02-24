#ifndef _WINDOWSSHAREDMEMORY_H
#define _WINDOWSSHAREDMEMORY_H

#include "AbstractionTypes.h"
#include <string>

class WindowsSharedMemory
{
public:
  WindowsSharedMemory(const std::string& name, uint32_t size = 0);
  ~WindowsSharedMemory();

  void* begin() const;
  void* end() const;

  uint32_t getSize() const;

private:
  // Private, undefined copy constructor and assignment operator so they can't be used
  WindowsSharedMemory(const WindowsSharedMemory&);
  WindowsSharedMemory& operator = (const WindowsSharedMemory&);

  static const std::string s_shmNameBase;
  const std::string m_shmName;
  Handle m_handle;
  void* m_shmBegin;
  uint32_t m_shmSize;
};

#endif
