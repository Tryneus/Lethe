#include "ThreadRegistry.h"
#include "mct/hash-map.hpp"

using namespace lethe;

bool ThreadRegistry::remove(const std::string& name)
{
  m_mutex.lock();

  for(auto j = m_threadArray.begin(); j != m_threadArray.cend(); ++j)
  {
    if(j->first == name)
    {
      m_threadArray.erase(j);
      break;
    }
  }

  auto i = m_threads->find(name);
  if(i == m_threads->cend())
    return false;

  Thread* thread = i->second;
  m_threads->erase(i);

  m_mutex.unlock();

  delete thread;
  return true;
}

Thread* ThreadRegistry::get(const std::string& name)
{
  m_mutex.lock();

  auto i = m_threads->find(name);

  if(i == m_threads->cend())
    return NULL;

  m_mutex.unlock();

  return i->second;
}

const std::vector<std::pair<std::string, Thread*> >& ThreadRegistry::getArray()
{
  return m_threadArray;
}

ThreadRegistry::ThreadRegistry()
{
  // Do nothing
}

ThreadRegistry::~ThreadRegistry()
{
  m_mutex.lock();

  // Stop all registered threads
  for(auto i = m_threads->begin(); i != m_threads->cend(); ++i)
    i->second->stop();

  // Delete all registered threads (each call will not return until stop has completed)
  while(!m_threads->empty())
    delete m_threads->begin()->second;

  m_mutex.unlock(); // TODO: if another thread is waiting to perform an operation, it will be working on a dead instance
}

bool ThreadRegistry::threadExists(const std::string& name) const
{
  return (m_threads->find(name) != m_threads->end());
}

void ThreadRegistry::addInternal(const std::string& name, Thread* thread)
{
  if(!m_threads->insert(std::make_pair(name, thread)).second)
    throw std::logic_error("failed to insert new thread '" + name + "' into map");

  m_threadArray.push_back(std::make_pair(name, thread));
}

