#include "Log.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include <iostream>
#include <unistd.h>

////////////////////////////////////
// Log implementation
////////////////////////////////////
Log::Log() :
  m_mutex(true),
  m_logLevel(Debug),
  m_statementLevel(Debug),
  m_handler(new StreamLogHandler(std::cout))
{
  m_mutex.unlock();
}

Log::~Log()
{
  m_mutex.lock();
  delete m_handler;
}

Log::Level Log::getLevel() const
{
  return m_logLevel;
}

void Log::setLevel(Log::Level level)
{
  m_mutex.lock();
  m_logLevel = level;
  m_mutex.unlock();
}

void Log::disable()
{
  m_mutex.lock();

  delete m_handler;
  m_handler = NULL;
  m_handler = new DisabledLogHandler();

  m_mutex.unlock();
}

void Log::setStreamMode(std::ostream& out)
{
  m_mutex.lock();

  delete m_handler;
  m_handler = NULL;
  m_handler = new StreamLogHandler(out);

  m_mutex.unlock();
}

void Log::setFileMode(const std::string& filename)
{
  m_mutex.lock();

  delete m_handler;
  m_handler = NULL;
  m_handler = new FileLogHandler(filename);

  m_mutex.unlock();
}

void Log::lock()
{
  m_mutex.lock();
}

void Log::unlock()
{
  m_mutex.unlock();
}

Log& Log::operator << (Log::Command c)
{
  if(c == Commit)
  {
    if(m_statementLevel <= m_logLevel)
      m_handler->write(m_statement.str());

    m_statement.str("");
    m_statementLevel = Debug;
  }
  else if(c == Time)
  {
    m_statement << getTimeString();
  }

  return *this;
}

Log& Log::operator << (Log::Level level)
{
  m_statementLevel = level;
  return *this;
}


// DisabledLogHandler implementation

#if defined(__GNUG__)
void Log::DisabledLogHandler::write(const std::string& statement __attribute__ ((unused)))
#else
void Log::DisabledLogHandler::write(const std::string& statement)
#endif
{
  // Do nothing
}


// StreamLogHandler implementation

Log::StreamLogHandler::StreamLogHandler(std::ostream& out) :
  m_out(out)
{
  // Do nothing
}

void Log::StreamLogHandler::write(const std::string& statement)
{
  if(::write(STDOUT_FILENO, (statement + "\n").c_str(), statement.length() + 1) <= 0)
    throw std::bad_syscall("write", lastError());
  fsync(STDOUT_FILENO);
  //  m_out << statement << std::endl;
}


// FileLogHandler implementation

Log::FileLogHandler::FileLogHandler(const std::string& filename) :
  m_filename(filename),
  m_out(m_filename.c_str(), std::ios_base::app)
{
  if(!m_out.good())
    throw std::runtime_error("Failed to open log file '" + m_filename + "'");
}

Log::FileLogHandler::~FileLogHandler()
{
  m_out.close();
}

void Log::FileLogHandler::write(const std::string& statement)
{
  if(!m_out.good())
    throw std::runtime_error("Log file '" + m_filename + "' not good");

  m_out << statement << std::endl;
}
