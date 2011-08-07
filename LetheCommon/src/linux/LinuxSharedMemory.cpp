#include "LetheTypes.h"
#include "LetheException.h"
#include "LetheFunctions.h"
#include "linux/LinuxSharedMemory.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

using namespace lethe;

const std::string LinuxSharedMemory::s_nameBase("/");
const mode_t LinuxSharedMemory::s_filePermissions(0666);

LinuxSharedMemory::LinuxSharedMemory(uint32_t size, const std::string& name) :
  m_fullName(s_nameBase + name),
  m_name(name),
  m_data(NULL),
  m_size(size)
{
  const int openFlags = O_RDWR | O_CREAT | O_EXCL;
  const Handle handle = shm_open(m_fullName.c_str(), openFlags, s_filePermissions);

  if(handle == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("shm_open", lastError());

  try
  {
    // Set the size of shared memory
    if(ftruncate(handle, m_size) != 0)
      throw std::bad_syscall("ftruncate", lastError());

    m_data = mmap(NULL, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);

    if(m_data == MAP_FAILED)
      throw std::bad_syscall("mmap", lastError());
  }
  catch(...)
  {
    close(handle);
    shm_unlink(m_fullName.c_str());
    throw;
  }

  close(handle);
}

LinuxSharedMemory::LinuxSharedMemory(const std::string& name) :
  m_fullName(s_nameBase + name),
  m_name(name),
  m_data(NULL),
  m_size(0)
{
  const int openFlags = O_RDWR;
  const Handle handle = shm_open(m_fullName.c_str(), openFlags, s_filePermissions);

  if(handle == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("shm_open", lastError());

  try
  {
    // Get the size of the shared memory file
    struct stat fileInfo;
    if(fstat(handle, &fileInfo) != 0)
      throw std::bad_syscall("fstat", lastError());

    m_size = fileInfo.st_size;

    m_data = mmap(NULL, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);

    if(m_data == MAP_FAILED)
      throw std::bad_syscall("mmap", lastError());
  }
  catch(...)
  {
    close(handle);
    shm_unlink(m_fullName.c_str());
    throw;
  }

  close(handle);
}

LinuxSharedMemory::~LinuxSharedMemory()
{
  shm_unlink(m_fullName.c_str());
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

uint32_t LinuxSharedMemory::size() const
{
  return m_size;
}

const std::string& LinuxSharedMemory::name() const
{
  return m_name;
}
