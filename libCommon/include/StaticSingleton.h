#ifndef _STATICSINGLETON_H
#define _STATICSINGLETON_H

#include <cstddef>

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

#endif
