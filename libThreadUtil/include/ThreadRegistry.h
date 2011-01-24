#ifndef _THREADREGISTRY_H
#define _THREADREGISTRY_H

#include "Abstraction.h"
#include "Singleton.h"

class ThreadRegistry : public Singleton<ThreadRegistry>
{
public:
  template <class ThreadType>
  ThreadType& add(const std::string& name);

  bool remove(const std::string& name);

  Thread* get(const std::string& name);
  std::vector<std::pair<std::string, Thread*> > getList();

private:
  friend class Singleton<ThreadRegistry>;

  ThreadRegistry();
  ~ThreadRegistry();

  std::map<std::string, Thread*> m_threadMap;
  Mutex m_controlMutex;
};

template <class ThreadType>
ThreadType& ThreadRegistry::add(const std::string& name)
{
  m_controlMutex.lock();

  std::map<std::string, Thread*>::iterator threadIter = m_threadMap.find(name);

  if(threadIter != m_threadMap.end())
  {
    m_controlMutex.unlock();
    throw Exception("Thread '" + name + "' already exists");
  }

  try
  {
    Thread& newThread = *(new ThreadType);
    m_threadMap.insert(std::make_pair<std::string, Thread*>(name, retval));

    m_controlMutex.unlock();
    return newThread;
  }
  catch(std::bad_alloc& ex)
  {
    m_controlMutex.unlock();
    throw;
  }
}

#endif