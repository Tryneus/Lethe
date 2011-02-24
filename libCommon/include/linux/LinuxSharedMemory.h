#ifndef _LINUXSHAREDMEMORY_H
#define _LINUXSHAREDMEMORY_H

#include "AbstractionTypes.h"
#include <sys/types.h>
#include <string>

class LinuxSharedMemory
{
public:
  LinuxSharedMemory(const std::string& name, uint32_t size = 0);
  ~LinuxSharedMemory();

  void* begin() const;
  void* end() const;

  uint32_t getSize() const;

private:
  // Private, undefined copy constructor and assignment operator so they can't be used
  LinuxSharedMemory(const LinuxSharedMemory&);
  LinuxSharedMemory& operator = (const LinuxSharedMemory&);

  static const std::string s_nameBase;
  static const mode_t s_filePermissions;

  const std::string m_name;
  void* m_data;
  uint32_t m_size;
};

#endif
