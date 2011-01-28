#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <string>

/*
 * The base exception class, can be used to catch any exception from libCommon.
 *  This is only used if an error occurs that the user must handle.
 */
class Exception
{
public:
  Exception(const std::string& message);

  const std::string& what() const;

private:
  std::string m_message;
};

/*
 * The OutOfMemoryException is used when the user attempts an operation that
 *  cannot be completed due to memory limitations, such as sending data towards
 *  a full pipe, or allocating a buffer in shared memory.
 */
class OutOfMemoryException : public Exception
{
public:
  OutOfMemoryException(const std::string& info);
};
#endif
