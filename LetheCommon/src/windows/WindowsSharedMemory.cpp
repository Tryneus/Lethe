#include "LetheTypes.h"
#include "LetheException.h"
#include "LetheFunctions.h"
#include "windows/WindowsSharedMemory.h"
#include <sstream>

using namespace lethe;

const std::string WindowsSharedMemory::s_shmBaseName("Local\\lethe-shm-");
WindowsAtomic WindowsSharedMemory::s_uniqueId(0);

WindowsSharedMemory::WindowsSharedMemory(uint32_t size) :
  m_data(NULL),
  m_size(size),
  m_handle(INVALID_HANDLE_VALUE)
{
  std::stringstream str;
  str << getProcessId() << "-" << s_uniqueId.increment();
  m_name.assign(str.str());

  m_handle = CreateFileMapping(INVALID_HANDLE_VALUE,
                               NULL,
                               PAGE_READWRITE,
                               0,
                               m_size + sizeof(uint32_t),
                               (s_shmBaseName + m_name).c_str());


  if(GetLastError() == ERROR_ALREADY_EXISTS)
  {
    // TODO: delete? automatically assigned name should be a leak from an old, dead process
    CloseHandle(m_handle);
    throw std::bad_syscall("CreateFileMapping", "Shared memory file already exists: " + m_name);
  }

  if(m_handle == NULL)
  {
    throw std::bad_syscall("CreateFileMapping", lastError());
  }

  m_data = mapShmFile(m_handle, m_size);

  // Write the size of shared memory into the first 4 bytes
  reinterpret_cast<uint32_t*>(m_data)[0] = m_size;

  if(getShmSize(m_data) < m_size)
  {
    UnmapViewOfFile(m_data);
    CloseHandle(m_handle);
    throw std::runtime_error("unexpected shared memory size");
  }
}

WindowsSharedMemory::WindowsSharedMemory(uint32_t size, const std::string& name) :
  m_name(name),
  m_data(NULL),
  m_size(size),
  m_handle(INVALID_HANDLE_VALUE)
{
  m_handle = CreateFileMapping(INVALID_HANDLE_VALUE,
                               NULL,
                               PAGE_READWRITE,
                               0,
                               m_size + sizeof(uint32_t),
                               (s_shmBaseName + m_name).c_str());


  if(GetLastError() == ERROR_ALREADY_EXISTS)
  {
    CloseHandle(m_handle);
    throw std::bad_syscall("CreateFileMapping", "Shared memory file already exists: " + s_shmBaseName + m_name);
  }

  if(m_handle == NULL)
  {
    throw std::bad_syscall("CreateFileMapping", lastError());
  }

  m_data = mapShmFile(m_handle, m_size);

  // Write the size of shared memory into the first 4 bytes
  reinterpret_cast<uint32_t*>(m_data)[0] = m_size;

  if(getShmSize(m_data) < m_size)
  {
    UnmapViewOfFile(m_data);
    CloseHandle(m_handle);
    throw std::runtime_error("unexpected shared memory size");
  }
}

WindowsSharedMemory::WindowsSharedMemory(const std::string& name) :
  m_name(name),
  m_data(NULL),
  m_size(0),
  m_handle(INVALID_HANDLE_VALUE)
{
  m_handle = OpenFileMapping(FILE_MAP_ALL_ACCESS,
                             false,
                             (s_shmBaseName + m_name).c_str());

  if(m_handle == NULL)
    throw std::bad_syscall("OpenFileMapping", lastError());

  m_data = mapShmFile(m_handle, m_size);

  // Read the size from the beginning of shared memory
  m_size = reinterpret_cast<uint32_t*>(m_data)[0];
}

WindowsSharedMemory::~WindowsSharedMemory()
{
  UnmapViewOfFile(m_data);
  CloseHandle(m_handle);
}

uint32_t WindowsSharedMemory::getShmSize(void* addr)
{
  MEMORY_BASIC_INFORMATION info;
  uint32_t resultSize = VirtualQuery(addr, &info, sizeof(info));

  if(resultSize == 0)
    throw std::bad_syscall("VirtualQuery", lastError());

  if(info.Type != MEM_MAPPED)
    throw std::runtime_error("invalid shared memory region type");

  return info.RegionSize;
}

void* WindowsSharedMemory::mapShmFile(Handle handle, uint32_t size)
{
  void* data = MapViewOfFile(handle,
                             FILE_MAP_ALL_ACCESS,
                             0,
                             0,
                             size);

  if(data == NULL)
  {
    std::string errorString = lastError();
    CloseHandle(handle);
    throw std::bad_syscall("MapViewOfFile", errorString);
  }

  return data;
}

void* WindowsSharedMemory::begin() const
{
  return reinterpret_cast<uint8_t*>(m_data) + sizeof(uint32_t);
}

void* WindowsSharedMemory::end() const
{
  return reinterpret_cast<uint8_t*>(m_data) + m_size + sizeof(uint32_t);
}

uint32_t WindowsSharedMemory::size() const
{
  return m_size;
}

const std::string& WindowsSharedMemory::name() const
{
  return m_name;
}
