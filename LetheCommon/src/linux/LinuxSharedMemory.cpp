#include "LetheTypes.h"
#include "LetheException.h"
#include "LetheFunctions.h"
#include "linux/LinuxSharedMemory.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>

using namespace lethe;

const uint32_t LinuxSharedMemory::s_maxSize(0x0FFFFFFF); // Set a maximum size of 256 MB
const uint32_t LinuxSharedMemory::s_magic(0x47A01B7C);
const std::string LinuxSharedMemory::s_baseName("/lethe-shm-");
LinuxAtomic LinuxSharedMemory::s_uniqueId(0);
const mode_t LinuxSharedMemory::s_filePermissions(0666);

LinuxSharedMemory::LinuxSharedMemory(uint32_t size) :
  m_data(NULL),
  m_size(size + sizeof(uint32_t))
{
  if(size > s_maxSize)
    throw std::invalid_argument("size");

  std::stringstream str;
  str << getProcessId() << "-" << s_uniqueId.increment();
  m_name.assign(str.str());

  const int openFlags = O_RDWR | O_CREAT | O_EXCL;
  const Handle handle = shm_open((s_baseName + m_name).c_str(), openFlags, s_filePermissions);

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

    // Indicate that shared memory is initialized
    reinterpret_cast<uint32_t*>(m_data)[0] = s_magic;
  }
  catch(...)
  {
    close(handle);
    shm_unlink((s_baseName + m_name).c_str());
    throw;
  }

  close(handle);
}

LinuxSharedMemory::LinuxSharedMemory(uint32_t size, const std::string& name) :
  m_name(name),
  m_data(NULL),
  m_size(size + sizeof(uint32_t))
{
  if(size > s_maxSize)
    throw std::invalid_argument("size");

  const int openFlags = O_RDWR | O_CREAT | O_EXCL;
  const Handle handle = shm_open((s_baseName + m_name).c_str(), openFlags, s_filePermissions);

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

    // Indicate that shared memory is initialized
    reinterpret_cast<uint32_t*>(m_data)[0] = s_magic;
  }
  catch(...)
  {
    close(handle);
    shm_unlink((s_baseName + m_name).c_str());
    throw;
  }

  close(handle);
}

LinuxSharedMemory::LinuxSharedMemory(const std::string& name) :
  m_name(name),
  m_data(NULL),
  m_size(0)
{
  const int openFlags = O_RDWR;
  const Handle handle = shm_open((s_baseName + m_name).c_str(), openFlags, s_filePermissions);

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
 
    // Check that shared memory has been initialized
    if(reinterpret_cast<uint32_t*>(m_data)[0] != s_magic)
    {
      // Sleep a little and try again, if it's still not done, fail
      sleep_ms(10);
      if(reinterpret_cast<uint32_t*>(m_data)[0] != s_magic)
      {
        close(handle);
        shm_unlink((s_baseName + m_name).c_str());
        munmap(m_data, m_size);
        throw std::runtime_error("shared memory not fully initialized");
      }
    }
  }
  catch(...)
  {
    close(handle);
    shm_unlink((s_baseName + m_name).c_str());
    throw;
  }

  close(handle);
}

LinuxSharedMemory::~LinuxSharedMemory()
{
  shm_unlink((s_baseName + m_name).c_str());
  munmap(m_data, m_size);
}

void* LinuxSharedMemory::begin() const
{
  return reinterpret_cast<uint8_t*>(m_data) + sizeof(uint32_t);
}

void* LinuxSharedMemory::end() const
{
  return reinterpret_cast<uint8_t*>(m_data) + m_size;
}

uint32_t LinuxSharedMemory::size() const
{
  return m_size - sizeof(uint32_t);
}

const std::string& LinuxSharedMemory::name() const
{
  return m_name;
}
