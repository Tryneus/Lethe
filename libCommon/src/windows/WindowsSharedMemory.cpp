#include "LetheTypes.h"
#include "LetheException.h"
#include "LetheFunctions.h"
#include "windows/WindowsSharedMemory.h"

using namespace lethe;

const std::string WindowsSharedMemory::s_nameBase("Global\\");

WindowsSharedMemory::WindowsSharedMemory(const std::string& name, uint32_t size) :
  m_name(s_shmNameBase + name),
  m_data(NULL),
  m_size(size),
  m_handle(INVALID_HANDLE_VALUE)
{
  if(m_size != 0) // Create new shared memory
  {
    m_handle = CreateFileMapping(INVALID_HANDLE_VALUE,
                                 NULL,
                                 PAGE_READWRITE,
                                 0,
                                 m_size,
                                 m_name.c_str());

    if(GetLastError() == ERROR_ALREADY_EXISTS)
    {
      CloseHandle(m_handle);
      throw std::bad_syscall("CreateFileMapping", "Shared memory file already exists: " + m_name);
    }
  }
  else // Open existing shared memory
  {
    m_handle = OpenFileMapping(FILE_MAP_ALL_ACCESS,
                               false,
                               m_name.c_str());
  }

  if(m_handle == NULL)
    throw std::bad_syscall(m_size ? "CreateFileMapping" : "OpenFileMapping", lastError());

  // Get and verify shared memory size
  m_size = GetFileSize(m_handle, NULL);

  if(m_size == INVALID_FILE_SIZE)
  {
    std::string errorString = getLastError();
    CloseHandle(m_handle);
    throw std::bad_syscall("GetFileSize", lastError());
  }
  else if(size != 0 && m_size != size)
  {
    throw std::runtime_error("Unexpected shared memory size")
  }

  // Map the shared memory file into process memory
  m_data = MapViewOfFile(m_handle,
                         FILE_MAP_ALL_ACCESS,
                         0,
                         0,
                         m_size);

  if(m_data == NULL)
  {
    std::string errorString = lastError();
    CloseHandle(m_handle);
    throw std::bad_syscall("MapViewOfFile", errorString);
  }
}

WindowsSharedMemory::~WindowsSharedMemory()
{
  UnmapViewOfFile(m_data);
  CloseHandle(m_handle);
}

void* WindowsSharedMemory::begin() const
{
  return m_data;
}

void* WindowsSharedMemory::end() const
{
  return reinterpret_cast<uint8_t*>(m_data) + m_size;
}

uint32_t WindowsSharedMemory::getSize() const
{
  return m_size;
}

const std::string& WindowsSharedMemory::name() const
{
  return m_name;
}
