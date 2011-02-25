#include "LetheException.h"

std::bad_syscall::bad_syscall(const std::string& syscall, const std::string& error) :
  m_message(syscall + " failed: " + error)
{
  // Do nothing
}

std::bad_syscall::~bad_syscall() throw()
{
  // Do nothing
}

const char* std::bad_syscall::what() const throw()
{
  return m_message.c_str();
}

