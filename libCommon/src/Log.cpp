#include "Log.h"
#include "Exception.h"
#include "Abstraction.h"
#include <iostream>

////////////////////////////////////
// Log implementation
////////////////////////////////////
Log::Log() :
  m_mutex(true),
  m_logLevel(Debug),
  m_statementLevel(Debug),
  m_handler(new StdoutLogHandler)
{
  m_mutex.unlock();
}

Log::~Log()
{
  m_mutex.lock();
  delete m_handler;
}

void Log::disable()
{
  m_mutex.lock();

  delete m_handler;
  m_handler = new DisabledLogHandler();

  m_mutex.unlock();
}

void Log::setStdoutMode(Log::Level level)
{
  m_mutex.lock();

  delete m_handler;
  m_handler = new StdoutLogHandler();
  m_logLevel = level;

  m_mutex.unlock();
}

void Log::setFileMode(const std::string& filename, Log::Level level)
{
  m_mutex.lock();

  delete m_handler;
  m_handler = new FileLogHandler(filename);
  m_logLevel = level;

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

////////////////////////////////////
// DisabledLogHandler implementation
////////////////////////////////////
void Log::DisabledLogHandler::write(const std::string& statement __attribute__ ((unused)))
{
  // Do nothing
}

////////////////////////////////////
// StdoutLogHandler implementation
////////////////////////////////////
void Log::StdoutLogHandler::write(const std::string& statement)
{
  std::cout << statement << std::endl;
}

////////////////////////////////////
// FileLogHandler implementation
////////////////////////////////////
Log::FileLogHandler::FileLogHandler(const std::string& filename) :
  m_filename(filename),
  m_out(m_filename.c_str(), std::ios_base::app)
{
  if(!m_out.good())
    throw Exception("Failed to open log file '" + m_filename + "'");
}

Log::FileLogHandler::~FileLogHandler()
{
  m_out.close();
}

void Log::FileLogHandler::write(const std::string& statement)
{
  if(!m_out.good())
    throw Exception("Log file '" + m_filename + "' not good");

  m_out << statement << std::endl;
}
