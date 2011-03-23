#ifndef _THREADREGISTRY_H
#define _THREADREGISTRY_H

#include "Lethe.h"
#include "Singleton.h"

// Prototype of the hash map, so users don't need the include
namespace mct
{
  template<typename, typename, typename, typename, typename, bool>
  class closed_hash_map;
}

namespace lethe
{
  class ThreadRegistry : public Singleton<ThreadRegistry>
  {
  public:
    template <class ThreadType>
    ThreadType& add(const std::string& name);

    Thread* get(const std::string& name);

    bool remove(const std::string& name);

    bool threadExists(const std::string& name) const;

    std::vector<std::pair<std::string, Thread*> > getList();

  private:
    friend class Singleton<ThreadRegistry>;

    ThreadRegistry();
    ~ThreadRegistry();

    void addInternal(const std::string& name, Thread* thread);

    mct::closed_hash_map<std::string,
                         Thread*,
                         std::tr1::hash<std::string>,
                         std::equal_to<std::string>,
                         std::allocator<std::pair<const std::string, Thread*> >,
                         false>* m_threads;

    Mutex m_mutex;
  };

  template <class ThreadType>
  ThreadType& ThreadRegistry::add(const std::string& name)
  {
    Thread* newThread = NULL;

    m_mutex.lock();

    if(threadExists(name))
      throw std::logic_error("thread '" + name + "' already exists");

    try
    {
      newThread = new ThreadType();
      addInternal(name, newThread);
    }
    catch(...)
    {
      m_mutex.unlock();
      delete newThread;
      throw;
    }

    m_mutex.unlock();
    return *newThread;
  }
}

#endif
