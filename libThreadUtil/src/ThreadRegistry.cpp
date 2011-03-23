#include "ThreadRegistry.h"
#include "mct/hash-map.hpp"

using namespace lethe;

bool ThreadRegistry::remove(const std::string& name)
{
  m_mutex.lock();

  mct::closed_hash_map<std::string, Thread*>::iterator i = m_threads->find(name);

  if(i == m_threads->end())
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

  mct::closed_hash_map<std::string, Thread*>::iterator i = m_threads->find(name);

  if(i == m_threads->end())
    return NULL;

  m_mutex.unlock();
  
  return i->second;
}

std::vector<std::pair<std::string, Thread*> > ThreadRegistry::getList()
{
  std::vector<std::pair<std::string, Thread*> > threads;

  m_mutex.lock();

  threads.reserve(m_threads->size());
  for(mct::closed_hash_map<std::string, Thread*>::iterator i = m_threads->begin(); i != m_threads->end(); ++i)
    threads.push_back(*i);

  m_mutex.unlock();

  return threads;
}

ThreadRegistry::ThreadRegistry()
{
  // Do nothing
}

ThreadRegistry::~ThreadRegistry()
{
  m_mutex.lock();

  // Stop all registered threads
  for(mct::closed_hash_map<std::string, Thread*>::iterator i = m_threads->begin(); i != m_threads->end(); ++i)
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
    throw std::logic_error("failed to insert new thread '" + name + "'");
}

