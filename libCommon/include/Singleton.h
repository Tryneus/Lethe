#ifndef _SINGLETON_H
#define _SINGLETON_H

#include <cstddef>
#include "AbstractionBasic.h"

/*
 * The Singleton template is provided as a thread-safe implementation of
 *  the singleton pattern.  This is meant to be used by deriving your class from
 *  this template, then providing your class as the template parameter.  It is also
 *  suggested that you make the derived class's constructor and destructor private
 *  and friend Singleton (so only Singleton will be able to instantiate or destroy
 *  an instance of your class).
 *
 * Example:
 *   class Foo : public Singleton<Foo>
 *   {
 *   private:
 *     friend class Singleton<Foo>;
 *     Foo();
 *   };
 */
template <class T>
class Singleton
{
public:
  static T& getInstance()
  {
    s_mutex.lock();
    if(s_instance == NULL)
      s_instance = new T;
    s_mutex.unlock();
    return *s_instance;
  };

  static void destroyInstance()
  {
    s_mutex.lock();
    delete s_instance;
    s_instance = NULL;
    s_mutex.unlock();
  };

  inline static bool exists()
  {
    return (s_instance != NULL);
  };

private:
  static T* s_instance;
  static Mutex s_mutex;
};

template <class T>
T* Singleton<T>::s_instance = NULL;

template <class T>
Mutex Singleton<T>::s_mutex;

#endif
