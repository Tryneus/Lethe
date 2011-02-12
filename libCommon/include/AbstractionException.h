#ifndef _ABSTRACTIONEXCEPTION_H
#define _ABSTRACTIONEXCEPTION_H

#include <string>
#include <exception>
#include <stdexcept>

namespace std
{

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
