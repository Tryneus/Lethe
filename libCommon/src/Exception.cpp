#include "Exception.h"

Exception::Exception(const std::string& message) :
  m_message(message)
{
  // Do nothing
}


const std::string& Exception::what() const
{
  return m_message;
}

OutOfMemoryException::OutOfMemoryException(const std::string& info) :
  Exception("Out of memory (" + info + ")")
{
  // Do nothing
}
