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

class OutOfMemoryException : public Exception
{
public:
  OutOfMemoryException(const std::string& info);
};
#endif
