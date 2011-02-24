#include "AbstractionTypes.h"
#include "AbstractionException.h"
#include "AbstractionFunctions.h"
#include "linux/LinuxSharedMemory.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>


const std::string LinuxSharedMemory::s_nameBase("/dev/shm/");
const mode_t LinuxSharedMemory::s_filePermissions(0666);

LinuxSharedMemory::LinuxSharedMemory(const std::string& name, uint32_t size) :
  m_name(s_nameBase + name),
  m_data(NULL),
  m_size(size)
{
  int handle = INVALID_HANDLE_VALUE;

  handle = shm_open(m_name.c_str(), O_RDWR | O_CREAT, s_filePermissions);

  if(handle == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("shm_open", lastError());

  if(m_size == 0) // Get the existing size
  {
    struct stat fileInfo;

    if(fstat(handle, &fileInfo) != 0)
    {
      std::string errorString = lastError();
      close(handle);
      shm_unlink(m_name.c_str());
      throw std::bad_syscall("fstat", errorString);
    }
    
    m_size = fileInfo.st_size;
  }
  else // Set the size
  {
    if(ftruncate(handle, m_size) != 0)
    {
      std::string errorString = lastError();
      close(handle);
      shm_unlink(m_name.c_str());
      throw std::bad_syscall("ftruncate", errorString);
    }
  }

  m_data = mmap(NULL, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);

  if(m_data == MAP_FAILED)
  {
    std::string errorString = lastError();
    close(handle);
    shm_unlink(m_name.c_str());
    throw std::bad_syscall("mmap", errorString);
  }

  close(handle);
}

LinuxSharedMemory::~LinuxSharedMemory()
{
  shm_unlink(m_name.c_str());
  munmap(m_data, m_size);
}

void* LinuxSharedMemory::begin() const
{
  return m_data;
}

void* LinuxSharedMemory::end() const
{
  return reinterpret_cast<uint8_t*>(m_data) + m_size;
}

uint32_t LinuxSharedMemory::getSize() const
{
  return m_size;
}
