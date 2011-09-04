#ifndef _LINUXSHAREDMEMORY_H
#define _LINUXSHAREDMEMORY_H

#include "LetheTypes.h"
#include "LinuxAtomic.h"
#include <sys/types.h>
#include <string>

namespace lethe
{
  class LinuxSharedMemory
  {
  public:
    explicit LinuxSharedMemory(uint32_t size);
    explicit LinuxSharedMemory(const std::string& name);
    LinuxSharedMemory(uint32_t size, const std::string& name);

    ~LinuxSharedMemory();

    void* begin() const;
    void* end() const;

    uint32_t size() const;
    const std::string& name() const;

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxSharedMemory(const LinuxSharedMemory&);
    LinuxSharedMemory& operator = (const LinuxSharedMemory&);

    static const uint32_t s_maxSize;
    static const uint32_t s_magic;
    static const mode_t s_filePermissions;
    static const std::string s_baseName;
    static LinuxAtomic s_uniqueId;

    std::string m_name;
    void* m_data;
    uint32_t m_size;
  };
}

#endif
