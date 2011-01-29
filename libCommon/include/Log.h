#ifndef _LOG_H
#define _LOG_H

#include "StaticSingleton.h"
#include "Abstraction.h"
#include <fstream>
#include <sstream>

/*
 * The Log class provides an ostream-style interface for writing log statements of
 *  various levels to different targets.  One target is selected at a time process-wide,
 *  and one log-level is selected, which may filter out statements of lower severity.
 *
 * This class is implemented as a static singleton, meaning that it is created during
 *  static initialization and destroyed when the process exits.  This means that as soon
 *  as the main() function begins, logging is available.  The default is to write to the
 *  console.  It is suggested to only use the Log* macros when writing a statement.
 *  Using the Log class directly is rather cumbersome and not encouraged.
 *
 * Example:
 *  LogInfo("text: " << value << ", more text");
 *  LogError("panic");
 *
 * The Log class uses LogHandler subclasses to define the behavior in different modes.
 *  At the moment, there are only modes for writing to the console, writing to a file,
 *  or disabling all log statements outright.
 *
 * In the future, this may be extended to that a user may provide their own log handlers
 *  to redirect output elsewhere or do other specialized tasks.
 */

// The LOG_BASE may be overridden by a user who knows what they're doing (before including this file)
#if !defined(LOG_BASE)
  #define LOG_BASE(level) level << Log::Time << ", " << __FILE__ << ":" << __LINE__ 
#endif

// TODO: Add if statements here to skip the log statement if that level is turned off at runtime
#ifndef DISABLE_LOG_ERROR
  #define LogError(a) do { Log& log = Log::getInstance(); \
                           log.lock(); \
                           log << LOG_BASE(Log::Error) << " Error - " << a << Log::Commit; \
                           log.unlock(); } while(0)
#else
  #define LogError(...) do { ; } while(0)
#endif

#ifndef DISABLE_LOG_INFO
  #define LogInfo(a)  do { Log& log = Log::getInstance(); \
                           log.lock(); \
                           log << LOG_BASE(Log::Info) << " Info - " << a << Log::Commit; \
                           log.unlock(); } while(0)
#else
  #define LogInfo(...) do { ; } while(0)
#endif

#ifndef DISABLE_LOG_DEBUG
  #define LogDebug(a) do { Log& log = Log::getInstance(); \
                           log.lock(); \
                           log << LOG_BASE(Log::Debug) << " Debug - " << a << Log::Commit; \
                           log.unlock(); } while(0)
#else
  #define LogDebug(...) do { ; } while(0)
#endif

class Log : public StaticSingleton<Log>
{
public:

  enum Level
  {
    Error = 0,
    Info = 1,
    Debug = 2,
    NumLevels = 3
  };

  void disable();
  void setStdoutMode(Level level = Debug);
  void setFileMode(const std::string& filename, Level level = Debug);

  void lock();
  void unlock();

  template <typename T>
  Log& operator << (const T& data);

  enum Command
  {
    Commit = 0,
    Time = 1
  };

  Log& operator << (Command c);
  Log& operator << (Level level);

private:
  friend class StaticSingleton<Log>;
  Log();
  ~Log();

  // Abstract base class for different styles of logging
  class LogHandler
  {
  public:
    virtual void write(const std::string& statement) = 0;
  };

  // Class for disabling logging, all output is built but discarded
  class DisabledLogHandler : public LogHandler
  {
  public:
    void write(const std::string& statement);
  };

  // Class for logging to standard output
  class StdoutLogHandler : public LogHandler
  {
  public:
    void write(const std::string& statement);
  };

  // Class for logging to a file
  class FileLogHandler : public LogHandler
  {
  public:
    FileLogHandler(const std::string& filename);
    virtual ~FileLogHandler();

    void write(const std::string& statement);

  private:
    const std::string m_filename;
    std::ofstream m_out;
  };

  Mutex m_mutex;
  Level m_logLevel;
  Level m_statementLevel;
  std::stringstream m_statement;
  LogHandler* m_handler;
};


// Note: locking should be done by the Log* macros (to keep lock/unlock cycles low)
template <typename T>
Log& Log::operator << (const T& data)
{
  m_statement << data;
  return *this;
}

#endif
