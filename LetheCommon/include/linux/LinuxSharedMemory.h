#ifndef _LINUXSHAREDMEMORY_H
#define _LINUXSHAREDMEMORY_H

#include "LetheTypes.h"
#include <sys/types.h>
#include <string>

namespace lethe
{
  class LinuxSharedMemory
  {
  public:
    LinuxSharedMemory(uint32_t size, const std::string& name);
    LinuxSharedMemory(const std::string& name);
    ~LinuxSharedMemory();

    void* begin() const;
    void* end() const;

    uint32_t size() const;
    const std::string& name() const;

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    LinuxSharedMemory(const LinuxSharedMemory&);
    LinuxSharedMemory& operator = (const LinuxSharedMemory&);

    static const std::string s_nameBase;
    static const mode_t s_filePermissions;

    const std::string m_fullName;
    const std::string m_name;
    void* m_data;
    uint32_t m_size;
  };
}

#endif
