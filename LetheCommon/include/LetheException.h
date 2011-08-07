#ifndef _LETHEEXCEPTION_H
#define _LETHEEXCEPTION_H

#include <string>
#include <exception>
#include <stdexcept>

namespace std // Not using lethe namespace since this is an extension of std::exception
{

  // An exception that will be thrown when a system call fails, containing the name of
  //  the system call and any error information that can be provided
  class bad_syscall : public std::exception
  {
  public:
    bad_syscall(const std::string& syscall, const std::string& error);
    virtual ~bad_syscall() throw();

    virtual const char* what() const throw();

  private:
    const std::string m_message;
  };

}

#endif
