#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <string>

class Exception
{
public:
  Exception(const std::string& message);

  const std::string& what() const;

private:
  std::string m_message;
};

#endif
