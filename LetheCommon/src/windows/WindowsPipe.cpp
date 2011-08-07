#include "windows/WindowsPipe.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include <Windows.h>
#include <sstream>
#include <algorithm>

using namespace lethe;

const std::string WindowsPipe::s_shmNameSuffix("-pipe-shm");
const std::string WindowsPipe::s_readEventNameSuffix("-pipe-read");
const std::string WindowsPipe::s_writeEventNameSuffix("-pipe-write");
const std::string WindowsPipe::s_readMutexNameSuffix("-pipe-read");
const std::string WindowsPipe::s_writeMutexNameSuffix("-pipe-write");
WindowsAtomic WindowsPipe::s_uniqueId(0);

WindowsPipe::WindowsPipe() :
  m_nameIn(generateName()),
  m_nameOut(m_nameIn),
  m_memoryIn(NULL),
  m_memoryOut(NULL),
  m_readEventIn(NULL),
  m_writeEventIn(NULL),
  m_readEventOut(NULL),
  m_writeEventOut(NULL),
  m_readMutexIn(NULL),
  m_readMutexOut(NULL),
  m_writeMutexIn(NULL),
  m_writeMutexOut(NULL),
  m_asyncThread(INVALID_HANDLE_VALUE),
  m_destructing(false)
{
  try
  {
    // One-way pipe, read/write uses the same objects
    m_memoryIn = new WindowsSharedMemory(sizeof(ShmLayout), m_nameIn + s_shmNameSuffix);
    m_memoryOut = m_memoryIn;

    m_readMutexIn = new WindowsMutex(false, m_nameIn + s_readMutexNameSuffix);
    m_readMutexOut = m_readMutexIn;
    m_writeMutexIn = new WindowsMutex(false, m_nameIn + s_writeMutexNameSuffix);
    m_writeMutexOut = m_writeMutexIn;

    m_readEventIn = new WindowsEvent(true, true, m_nameIn + s_readEventNameSuffix);
    m_readEventOut = m_readEventIn;
    m_writeEventIn = new WindowsEvent(false, false, m_nameIn + s_writeEventNameSuffix);
    m_writeEventOut = m_writeEventIn;
  }
  catch(...)
  {
    cleanup();
    throw;
  }

  // Initialize shared memory
  initializeShm(m_memoryOut->begin());
  initializeShm(m_memoryIn->begin());
}

WindowsPipe::WindowsPipe(const std::string& nameIn,
                         bool createIn,
                         const std::string& nameOut,
                         bool createOut) :
  m_nameIn(nameIn),
  m_nameOut(nameOut),
  m_memoryIn(NULL),
  m_memoryOut(NULL),
  m_readEventIn(NULL),
  m_writeEventIn(NULL),
  m_readEventOut(NULL),
  m_writeEventOut(NULL),
  m_readMutexIn(NULL),
  m_readMutexOut(NULL),
  m_writeMutexIn(NULL),
  m_writeMutexOut(NULL),
  m_asyncThread(INVALID_HANDLE_VALUE)
{
  try
  {
    // Try to open existing, then create a new one if that doesn't work
    try
    {
      if(createIn)
      {
        m_memoryIn = new WindowsSharedMemory(sizeof(ShmLayout), m_nameIn + s_shmNameSuffix);
        initializeShm(m_memoryIn->begin());
      }
    }
    catch(std::bad_syscall&) { }

    if(m_memoryIn == NULL)
      m_memoryIn = new WindowsSharedMemory(m_nameIn + s_shmNameSuffix);

    m_readMutexIn = new WindowsMutex(false, m_nameIn + s_readMutexNameSuffix);
    m_writeMutexIn = new WindowsMutex(false, m_nameIn + s_writeMutexNameSuffix);
    m_readEventIn = new WindowsEvent(true, true, m_nameIn + s_readEventNameSuffix);
    m_writeEventIn = new WindowsEvent(false, false, m_nameIn + s_writeEventNameSuffix);

    try
    {
      if(createOut)
      {
        m_memoryOut = new WindowsSharedMemory(sizeof(ShmLayout), m_nameOut + s_shmNameSuffix);
        initializeShm(m_memoryOut->begin());
      }
    }
    catch(std::bad_syscall&) { }

    if(m_memoryOut == NULL)
      m_memoryOut = new WindowsSharedMemory(m_nameOut + s_shmNameSuffix);
      
    m_readMutexOut = new WindowsMutex(false, m_nameOut + s_readMutexNameSuffix);
    m_writeMutexOut = new WindowsMutex(false, m_nameOut + s_writeMutexNameSuffix);
    m_readEventOut = new WindowsEvent(true, true, m_nameOut + s_readEventNameSuffix);
    m_writeEventOut = new WindowsEvent(false, false, m_nameOut + s_writeEventNameSuffix);
  }
  catch(std::exception&)
  {
    cleanup();
    throw;
  }
}

WindowsPipe::~WindowsPipe()
{
  // Stop the thread, if it exists
  m_destructing = true;

  if(m_asyncThread != INVALID_HANDLE_VALUE)
  {
    // Cancel the async write and wait for the thread to finish
    m_readEventOut->set();
    WaitForObject(m_asyncThread);
  }

  cleanup();
}

void WindowsPipe::initializeShm(void* shm)
{
  ShmLayout* pipeData = reinterpret_cast<ShmLayout*>(shm);

   pipeData->pendingWrite = false;
   pipeData->inOffset = 0;
   pipeData->outOffset = 0;
}

void WindowsPipe::cleanup()
{
  // In case of a one-way pipe, avoid double-free
  if(m_memoryOut != m_memoryIn)
    delete m_memoryOut;

  if(m_readEventOut != m_readEventIn)
    delete m_readEventOut;

  if(m_writeEventOut != m_writeEventIn)
    delete m_writeEventOut;

  if(m_readMutexOut != m_readMutexIn)
    delete m_readMutexOut;

  if(m_writeMutexOut != m_writeMutexIn)
    delete m_writeMutexOut;

  delete m_memoryIn;
  delete m_readEventIn;
  delete m_writeEventIn;
  delete m_readMutexIn;
  delete m_writeMutexIn;
}

void WindowsPipe::send(const void* buffer, uint32_t bufferSize)
{
  ShmLayout* pipeData = reinterpret_cast<ShmLayout*>(m_memoryOut->begin());

  if(getFreeSpace(pipeData) >= bufferSize)
  {
    m_writeMutexOut->lock();
    // Enough free space for the buffer, copy it over
    writeBuffer(buffer, bufferSize);

    // Set the incoming data event
    m_readMutexOut->lock();
    if(getUsedSpace(pipeData) > 0)
      m_writeEventOut->set();
    m_readMutexOut->unlock();

    m_writeMutexOut->unlock();
  }
  else
  {
    m_readMutexOut->lock();

    if(pipeData->pendingWrite)
    {
      m_readMutexOut->unlock();
      throw std::bad_alloc();
    }

    pipeData->pendingWrite = true;

    // Not enough space, start an async thread
    AsyncData* params = new AsyncData;
    params->buffer = reinterpret_cast<const char*>(buffer);
    params->size = bufferSize;
    params->instance = this;

    m_asyncThread = CreateThread(NULL, 0, asyncThreadHook, params, 0, NULL);

    m_readMutexOut->unlock();
  }
}

uint32_t WindowsPipe::receive(void* buffer, uint32_t bufferSize)
{
  ShmLayout* pipeData = reinterpret_cast<ShmLayout*>(m_memoryIn->begin());
  uint32_t bytesRead = 0;

  if(bufferSize != 0)
  {
    m_readMutexIn->lock();

    uint32_t usedSpace = getUsedSpace(pipeData);

    bytesRead = (usedSpace < bufferSize) ? usedSpace : bufferSize;

    if(bytesRead > 0)
    {
      if(pipeData->inOffset + bytesRead > sizeof(pipeData->data))
      {
        uint32_t splitSize = sizeof(pipeData->data) - pipeData->inOffset;
        memcpy(buffer, &pipeData->data[pipeData->inOffset], splitSize);
        memcpy(reinterpret_cast<uint8_t*>(buffer) + splitSize, &pipeData->data[0], bytesRead - splitSize);
        pipeData->inOffset = bytesRead - splitSize;
      }
      else
      {
        memcpy(buffer, &pipeData->data[pipeData->inOffset], bytesRead);
        pipeData->inOffset += bytesRead;
      }
      
      m_readEventIn->set();
    }

    // Reset the incoming data event
    if(getUsedSpace(pipeData) == 0)
      m_writeEventIn->reset();

    m_readMutexIn->unlock();
  }

  return bytesRead;
}

const std::string WindowsPipe::generateName()
{
  std::stringstream name;
  name << getProcessId() << "-" << s_uniqueId.increment();
  return name.str();
}

uint32_t WindowsPipe::getFreeSpace(ShmLayout* shm)
{
  // Get the data into local variables so it doesn't change during the function
  uint32_t inOffset = shm->inOffset;
  uint32_t outOffset = shm->outOffset;
  uint32_t freeSpace = inOffset - outOffset - 1;

  if(inOffset <= outOffset)
    freeSpace += sizeof(shm->data);

  return freeSpace;
}

uint32_t WindowsPipe::getUsedSpace(ShmLayout* shm)
{
  // Get the data into local variables so it doesn't change during the function
  uint32_t inOffset = shm->inOffset;
  uint32_t outOffset = shm->outOffset;
  uint32_t usedSpace = outOffset - inOffset;

  if(outOffset < inOffset)
    usedSpace += sizeof(shm->data);

  return usedSpace;
}

void WindowsPipe::writeBuffer(const void* buffer, uint32_t size)
{
  ShmLayout* pipeData = reinterpret_cast<ShmLayout*>(m_memoryOut->begin());

  if(pipeData->outOffset + size > sizeof(pipeData->data))
  {
    uint32_t splitSize = sizeof(pipeData->data) - pipeData->outOffset;
    memcpy(&pipeData->data[pipeData->outOffset], buffer, splitSize);
    memcpy(&pipeData->data[0], reinterpret_cast<const uint8_t*>(buffer) + splitSize, size - splitSize);
    pipeData->outOffset = size - splitSize;
  }
  else
  {
    memcpy(&pipeData->data[pipeData->outOffset], buffer, size);
    pipeData->outOffset += size;
  }
}

DWORD WINAPI WindowsPipe::asyncThreadHook(void* p)
{
  AsyncData* params = reinterpret_cast<AsyncData*>(p);
  params->instance->asyncThread(params->buffer, params->size);

  return 0;
}

void WindowsPipe::asyncThread(const char* buffer, uint32_t size)
{
  m_writeMutexOut->lock();
  ShmLayout* pipeData = reinterpret_cast<ShmLayout*>(m_memoryOut->begin());

  while(size > 0 && !m_destructing)
  {
    uint32_t freeSpace = getFreeSpace(pipeData);

    if(freeSpace > 0)
    {
      if(freeSpace >= size)
      {
        writeBuffer(buffer, size);
        size = 0;
      }
      else
      {
        writeBuffer(buffer, freeSpace);
        buffer = &buffer[freeSpace];
        size -= freeSpace;
      }

      // Set the outgoing data event
      m_readMutexOut->lock();
      if(getUsedSpace(pipeData) > 0)
        m_writeEventOut->set();
      m_readMutexOut->unlock();
    }

    if(WaitForObject(*m_readEventOut) != WaitSuccess)
      break;
  }

  m_readMutexOut->lock();
  m_asyncThread = INVALID_HANDLE_VALUE;
  pipeData->pendingWrite = false;
  m_readMutexOut->unlock();

  m_writeMutexOut->unlock();
}

WindowsPipe::operator WaitObject&()
{
  return *m_writeEventIn;
}

Handle WindowsPipe::getHandle() const
{
  return m_writeEventIn->getHandle();
}

bool WindowsPipe::flush(uint32_t timeout)
{
  bool retval = true;

  if(m_asyncThread != INVALID_HANDLE_VALUE)
  {
    if(WaitForObject(m_asyncThread, timeout) == WaitTimeout)
      retval = false;
  }

  return retval;
}