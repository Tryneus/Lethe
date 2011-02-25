#ifndef _STATICSINGLETON_H
#define _STATICSINGLETON_H

#include <cstddef>

/*
 * The StaticSingleton template is provided as a thread-safe implementation of
 *  the singleton pattern.  This is meant to be used by deriving your class from
 *  this template, then providing your class as the template parameter.  It is also
 *  suggested that you make the derived class's constructor and destructor private
 *  and friend Singleton (so only Singleton will be able to instantiate or destroy
 *  an instance of your class).
 *
 * The StaticSingleton class differs from the Singleton class in that the instance
 *  of this is instantiated at static initialization (before your main() function
 *  runs).  There are not many applications for this, but it saves some checks needed
 *  in getInstance.  This is used by the libCommon::Log class.  You should be careful
 *  when using StaticSingletons that you do not try to access it during static
 *  initialization before it has been created.
 *
 * Example:
 *  class Foo : public StaticSingleton<Foo>
 *  {
 *  private:
 *    friend class StaticSingleton<Foo>;
 *    Foo();
 *  };
 */
namespace lethe
{
  template <class T>
  class StaticSingleton
  {
  public:
    static T& getInstance()
    {
      return s_instance;
    };

  private:
    static T s_instance;
  };

  template <class T>
  T StaticSingleton<T>::s_instance;
}

#endif
