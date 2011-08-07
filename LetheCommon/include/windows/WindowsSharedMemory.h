#ifndef _WINDOWSSHAREDMEMORY_H
#define _WINDOWSSHAREDMEMORY_H

#include "LetheTypes.h"
#include "WindowsAtomic.h"
#include <string>

namespace lethe
{
  class WindowsSharedMemory
  {
  public:
    explicit WindowsSharedMemory(uint32_t size);
    explicit WindowsSharedMemory(const std::string& name);
    WindowsSharedMemory(uint32_t size, const std::string& name);

    ~WindowsSharedMemory();

    void* begin() const;
    void* end() const;

    uint32_t size() const;
    const std::string& name() const;

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsSharedMemory(const WindowsSharedMemory&);
    WindowsSharedMemory& operator = (const WindowsSharedMemory&);

    static const std::string s_shmBaseName;
    static WindowsAtomic s_uniqueId;

    static uint32_t getShmSize(void* addr);
    static void* mapShmFile(Handle handle, uint32_t size);

    std::string m_name;
    Handle m_handle;
    void* m_data;
    uint32_t m_size;
  };
}

#endif
