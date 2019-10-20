/*
* Copyright (c) 2009-2019 Brian Waters
* Copyright (c) 2019 Sprint
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __elogger_h_included
#define __elogger_h_included

/// @file
/// @brief Defines the logging related classes.

#include <vector>
#include <map>
#include <unordered_map>

/// @cond DOXYGEN_EXCLUDE
#ifdef minor
#undef minor
#endif

#ifdef major
#undef major
#endif

#ifdef SPDLOG_LEVEL_NAMES
#undef SPDLOG_LEVEL_NAMES
#endif

//#define SPDLOG_LEVEL_NAMES { "trace", "debug", "info",  "warning", "error", "critical", "off" };
#define SPDLOG_LEVEL_NAMES { "debug", "info", "startup", "minor", "major", "critical", "off" };

#define SPDLOG_ENABLE_SYSLOG
/// @endcond

#include "spdlog/spdlog.h"
#include "spdlog/async.h"

#include "ebase.h"
#include "eerror.h"
#include "egetopt.h"
#include "estring.h"
#include "eutil.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE
DECLARE_ERROR_ADVANCED2(ELoggerError_LogNotFound);
DECLARE_ERROR_ADVANCED2(ELoggerError_LogExists);
DECLARE_ERROR_ADVANCED2(ELoggerError_SinkSetNotFound);
DECLARE_ERROR_ADVANCED2(ELoggerError_SinkSetExists);

DECLARE_ERROR(ELoggerError_SinkSetLogIdNotSpecified);
DECLARE_ERROR(ELoggerError_SinkSetSinkIdNotSpecified);
DECLARE_ERROR(ELoggerError_SinkSetCategoryNotSpecified);
DECLARE_ERROR(ELoggerError_SinkSetSinkTypeNotSpecified);

DECLARE_ERROR_ADVANCED3(ELoggerError_SinkSetCreatePath);
DECLARE_ERROR_ADVANCED4(ELoggerError_SinkSetNotDirectory);
DECLARE_ERROR_ADVANCED4(ELoggerError_SinkSetUnrecognizedSinkType);
/// @endcond

class ELoggerSinkSet;

/// @brief Defines a logger.
class ELogger
{
   friend class ELoggerInit;

public:
   /// @brief Defines the various log levels.
   enum LogLevel
   {
      /// Debug
      eDebug      = spdlog::level::trace,
      /// Informational
      eInfo       = spdlog::level::debug,
      /// Startup
      eStartup    = spdlog::level::info,
      /// Minor
      eMinor      = spdlog::level::warn,
      /// Major
      eMajor      = spdlog::level::err,
      /// Critical
      eCritical   = spdlog::level::critical,
      /// Disables logging
      eOff        = spdlog::level::off
   };

   /// @brief Class constructor.
   /// @param logid the log ID for this logger.
   /// @param category the category description for this log.
   /// @param sinkid identifies the set of output sinks for this logger.
   ELogger(Int logid, cpStr category, Int sinkid);
   /// @brief Class destructor
   ~ELogger() {}

   /// @brief Retrieves the application name.
   /// @return the application name.
   static EString &applicationName() { return m_appname; }
   /// @brief Assigns the application name.
   /// @return the application name.
   static EString &applicationName(cpStr app) { return m_appname = app; }

   /// @brief Manually creates a logger.
   /// @param logid the log ID for this logger.
   /// @param category the category description for this log.
   /// @param sinkid identifies the set of output sinks for this logger.
   /// @return the newly created logger.
   /// @throws ELoggerError_LogExists
   /// @throws ELoggerError_SinkSetNotFound
   static ELogger &createLog(Int logid, cpStr category, Int sinkid);
   /// @brief Manually creates a sink set.
   /// @param sinkid the sink ID to assign to this sink set.
   /// @return the newly created sink set object.
   /// @throws ELoggerError_SinkSetExists
   static ELoggerSinkSet &createSinkSet(Int sinkid);

   /// @brief Retrieves the requested ELogger object.
   /// @param logid the log ID for the logger.
   /// @return the logger.
   /// @throws ELoggerError_LogNotFound
   static ELogger &log(Int logid)
   {
      auto srch = m_logs.find(logid);
      if (srch == m_logs.end())
         throw ELoggerError_LogNotFound(logid);
      return *srch->second;
   }

   /// @brief Retrieves the requested sink set object.
   /// @param sinkid the sink set ID.
   /// @return the sink set object.
   /// @throws ELoggerError_SinkSetNotFound
   static ELoggerSinkSet &sinkSet(Int sinkid)
   {
      if (m_sinksets.find(sinkid) == m_sinksets.end())
         throw ELoggerError_SinkSetNotFound(sinkid);
      
      return *m_sinksets[sinkid];
   }

   /// @brief Writes a debug message to this logger.
   /// @param format the format of the log message.
   /// @param args any arguments that will be substituted into the log format.
   template<typename... Args>
   Void debug( cpStr format, const Args &... args) { m_log->trace(format, args...); }
   /// @brief Writes a info message to this logger.
   /// @param format the format of the log message.
   /// @param args any arguments that will be substituted into the log format.
   template<typename... Args>
   Void info( cpStr format, const Args &... args) { m_log->debug(format, args...); }
   /// @brief Writes a startup message to this logger.
   /// @param format the format of the log message.
   /// @param args any arguments that will be substituted into the log format.
   template<typename... Args>
   Void startup( cpStr format, const Args &... args) { m_log->info(format, args...); }
   /// @brief Writes a minor message to this logger.
   /// @param format the format of the log message.
   /// @param args any arguments that will be substituted into the log format.
   template<typename... Args>
   Void minor( cpStr format, const Args &... args) { m_log->warn(format, args...); }
   /// @brief Writes a major message to this logger.
   /// @param format the format of the log message.
   /// @param args any arguments that will be substituted into the log format.
   template<typename... Args>
   Void major( cpStr format, const Args &... args) { m_log->error(format, args...); }
   /// @brief Writes a critical message to this logger.
   /// @param format the format of the log message.
   /// @param args any arguments that will be substituted into the log format.
   template<typename... Args>
   Void critical( cpStr format, const Args &... args) { m_log->critical(format, args...); }

   /// @brief Flushes any unwritten log messages to the underlying sinks.
   Void flush() { m_log->flush(); }

   /// @brief Assign a log level for this logger.  Any log messages lower than the specified log level will not be written.
   /// @param lvl the log level to assign.
   Void setLogLevel( LogLevel lvl ) { m_log->set_level((spdlog::level::level_enum)lvl); }
   /// @brief Retrieve the currently assigned log level.
   /// @return the current log level.
   LogLevel getLogLevel() { return (LogLevel)m_log->level(); }

   /// @brief Retrieve the name (category) of the logger.
   /// @return the logger name.
   const std::string & get_name() { return m_log->name(); }

   /// @brief Retrieve the current log level from the underlying spdlog object.
   /// @return the spdlog log level.
   spdlog::level::level_enum get_level() { return m_log->level(); }
   /// @brief Sets the current log level of the underlying spdlog object.
   /// @param lvl the spdlog log level.
   Void set_level(spdlog::level::level_enum lvl) { m_log->set_level(lvl); }

   /// @brief Retrieve the defined loggers.
   /// @return the logger collection.
   const std::map<std::string,std::shared_ptr<ELogger>> get_loggers() { return m_logs_by_name; }

protected:
   /// @brief Initilizes the logs from the configuration file.
   /// @param opt the options that contain the settings loaded from the configuration file.
   /// @throws ELoggerError_SinkSetLogIdNotSpecified
   /// @throws ELoggerError_SinkSetSinkIdNotSpecified
   /// @throws ELoggerError_SinkSetCategoryNotSpecified
   static Void init(EGetOpt &opt);
   /// @brief Shuts down the logging system.
   static Void uninit();

private:
   static EString m_appname;
   static std::unordered_map<Int,std::shared_ptr<ELoggerSinkSet>> m_sinksets;
   static std::unordered_map<Int,std::shared_ptr<ELogger>> m_logs;
   static std::map<std::string,std::shared_ptr<ELogger>> m_logs_by_name;

   static Void verifyPath(cpStr filename);

   Int m_logid;
   Int m_sinkid;
   EString m_category;
   std::shared_ptr<spdlog::async_logger> m_log;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Represents a logger output sink.
class ELoggerSink
{
public:
   /// @brief Defines the type of output sinks.
   enum SinkType
   {
      /// syslog
      eSyslog,
      /// standard output
      eStdout,
      /// standard error
      eStderr,
      /// a simple file
      eBasicFile,
      /// a rotating set of files
      eRotatingFile,
      /// a file that rolls over each day at a specified time
      eDailyFile
   };

   /// @brief Class destructor
   virtual ~ELoggerSink() {}

   /// @brief Retrieves the sink type.
   /// @return the sink type.
   SinkType getSinkType() { return m_sinktype; }
   /// @brief Retrieves the log level for this sink.
   /// @return the sink log level.
   ELogger::LogLevel getLogLevel() { return (ELogger::LogLevel)m_sinkptr->level(); }
   /// @brief Retrieves the log message formatting pattern.
   /// @return the log message formatting pattern.
   EString &getPattern() { return m_pattern; }

   /// @brief Assigns the sink log level.
   /// @param loglevel the new sink log level.
   /// @return the assigned sink log level.
   ELogger::LogLevel setLogLevel( ELogger::LogLevel loglevel )
      { m_sinkptr->set_level( (spdlog::level::level_enum)loglevel ); m_loglevel = loglevel; return getLogLevel(); }
   /// @brief Assigns the log message formatting pattern.
   /// @param pattern the log message formatting pattern.
   /// @return the log message formatting pattern.
   EString &setPattern( cpStr pattern )
      { m_pattern = pattern; m_sinkptr->set_pattern( m_pattern ); return getPattern(); }

   /// @brief Retrieves the underlying spdlog sink pointer.
   /// @return the underlying spdlog sink pointer.
   spdlog::sink_ptr getSinkPtr() { return m_sinkptr; }

   /// @brief Retrieves the default log message formatting pattern.
   /// @return the default log message formatting pattern.
   static EString &getDefaultPattern() { return m_defaultpattern; }

protected:
   /// @brief The base class constructor.
   /// @param sinktype the sink type.
   /// @param loglevel the log level for this sink.
   /// @param pattern the log message formatting pattern.
   ELoggerSink( SinkType sinktype, ELogger::LogLevel loglevel, cpStr pattern )
      : m_sinktype( sinktype ),
        m_loglevel( loglevel ),
        m_pattern( pattern )
   {
   }

   /// @brief Assigns the spdlog pointer value.
   /// @param sinkptr the spdlog pointer value.
   /// @return the spdlog pointer value.
   spdlog::sink_ptr setSinkPtr( spdlog::sink_ptr &sinkptr ) { return m_sinkptr = sinkptr; }

private:
   ELoggerSink();
   static EString m_defaultpattern;

   SinkType m_sinktype;
   ELogger::LogLevel m_loglevel;
   EString m_pattern;
   spdlog::sink_ptr m_sinkptr;
};

/// @brief A syslog sink.
class ELoggerSinkSyslog : public ELoggerSink
{
public:
   /// @brief Class constructor.
   /// @param loglevel the sink log level.
   /// @param pattern the log message formatting pattern.
   ELoggerSinkSyslog( ELogger::LogLevel loglevel, cpStr pattern );
   /// @brief Class destructor.
   virtual ~ELoggerSinkSyslog() {}
private:
   ELoggerSinkSyslog();
};

/// @brief A standard output sink.
class ELoggerSinkStdout : public ELoggerSink
{
public:
   /// @brief Class constructor.
   /// @param loglevel the sink log level.
   /// @param pattern the log message formatting pattern.
   ELoggerSinkStdout( ELogger::LogLevel loglevel, cpStr pattern );
   /// @brief Class destructor.
   virtual ~ELoggerSinkStdout() {}
private:
   ELoggerSinkStdout();
};

/// @brief A standard error sink.
class ELoggerSinkStderr : public ELoggerSink
{
public:
   /// @brief Class constructor.
   /// @param loglevel the sink log level.
   /// @param pattern the log message formatting pattern.
   ELoggerSinkStderr( ELogger::LogLevel loglevel, cpStr pattern );
   /// @brief Class destructor.
   virtual ~ELoggerSinkStderr() {}
private:
   ELoggerSinkStderr();
};

/// @brief A basic file sink.
class ELoggerSinkBasicFile : public ELoggerSink
{
public:
   /// @brief Class constructor.
   /// @param loglevel the sink log level.
   /// @param pattern the log message formatting pattern.
   /// @param filename the log file name.
   /// @param truncate indicates if the log file should be truncated if it already exists.
   ELoggerSinkBasicFile( ELogger::LogLevel loglevel, cpStr pattern,
      cpStr filename, Bool truncate );
   /// @brief Class destructor.
   virtual ~ELoggerSinkBasicFile() {}

   /// @brief Retrieves the log file name.
   /// @return the log file name.
   EString &getFilename() { return m_filename; }
   /// @brief Retrieves the truncate flag.
   /// @return True will cause the file to truncated when it is opened, otherwise False.
   Bool getTruncate() { return m_truncate; }

private:
   EString m_filename;
   Bool m_truncate;
};

/// @brief A rotating file sink.
class ELoggerSinkRotatingFile : public ELoggerSink
{
public:
   /// @brief Class constructor.
   /// @param loglevel the sink log level.
   /// @param pattern the log message formatting pattern.
   /// @param filename the log file name.
   /// @param maxsizemb the maximum size of a log file in megabytes.
   /// @param maxfiles the maximum number of files in the rotation.
   /// @param rotateonopen determines if the next file will be written to when the logger is opened.
   ELoggerSinkRotatingFile( ELogger::LogLevel loglevel, cpStr pattern,
      cpStr filename, size_t maxsizemb, size_t maxfiles, Bool rotateonopen );
   /// @brief Class destructor.
   virtual ~ELoggerSinkRotatingFile() {}

   /// @brief Retrieves the log file name.
   /// @return the log file name.
   EString &getFilename() { return m_filename; }
   /// @brief Retrieves the maximum log file size.
   /// @return the maximum log file size.
   size_t getMaxSizeMB() { return m_maxsizemb; }
   /// @brief Retrieves the maximum number of files in the rotation.
   /// @return the maximum number of files in the rotation.
   size_t getMaxFiles() { return m_maxfiles; }
   /// @brief Retrieves the rotate on open setting.
   /// @return True will cause the next file to be opened (regardless of it's size), otherwise the last file will be written to.
   Bool getRotateOnOpen() { return m_rotateonopen; }

private:
   EString m_filename;
   size_t m_maxsizemb;
   size_t m_maxfiles;
   Bool m_rotateonopen;
};

/// @brief A daily file sink.
class ELoggerSinkDailyFile : public ELoggerSink
{
public:
   /// @brief Class constructor.
   /// @param loglevel the sink log level.
   /// @param pattern the log message formatting pattern.
   /// @param filename the log file name.
   /// @param truncate indicates if the log file should be truncated if it already exists.
   /// @param rolloverhour the hour during the day when the rollover will occur.
   /// @param rolloverminute the minute within the hour when the rollover will occur.
   ELoggerSinkDailyFile( ELogger::LogLevel loglevel, cpStr pattern,
      cpStr filename, Bool truncate, Int rolloverhour, Int rolloverminute );
   /// @brief Class destructor.
   virtual ~ELoggerSinkDailyFile() {}

   /// @brief Retrieves the log file name.
   /// @return the log file name.
   EString &getFilename() { return m_filename; }
   /// @brief Retrieves the truncate flag.
   /// @return True will cause the file to truncated when it is opened, otherwise False.
   Bool getTruncate() { return m_truncate; }
   /// @brief Retrieves the hour during the day when the rollover will occur.
   /// @return the hour during the day when the rollover will occur.
   Int getRolloverHour() { return m_rolloverhour; }
   /// @brief Retrieves the minute within the hour when the rollover will occur.
   /// @return the minute within the hour when the rollover will occur.
   Int getRolloverMinute() { return m_rolloverminute; }

private:
   EString m_filename;
   Bool m_truncate;
   Int m_rolloverhour;
   Int m_rolloverminute;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Defines a group of output sinks that can be assigned to a logger.
class ELoggerSinkSet
{
   friend ELogger;
public:
   /// @brief Class constructor.
   /// @param id the sink set identifier.  If -1, then the next sink set ID will be assigned.
   ELoggerSinkSet(Int id=-1) : m_id(id) {}
   /// @brief Class destructor.
   ~ELoggerSinkSet() {}

   /// @brief Adds a sink to the sink set.
   /// @param sink the sink to add.
   Void addSink(std::shared_ptr<ELoggerSink> &sink)
   {
      m_sinks.push_back( sink );
      m_spdlog_sinks.push_back( sink->getSinkPtr() );
   }

   /// @brief Retrieves the vector of sinks.
   /// @return the vector of sinks.
   std::vector<std::shared_ptr<ELoggerSink>> &getVector() { return m_sinks; }
   /// @brief Retrieves the spdlog vector of sinks.
   /// @return the spdlog vector of sinks;
   std::vector<spdlog::sink_ptr> &getSpdlogVector() { return m_spdlog_sinks; }

   /// @brief Assigns the sink set ID.
   /// @param id the sink set ID.
   /// @return the sink set ID.
   Int setId(Int id) { return m_id = id; }
   /// @brief Retrieves teh sink set ID.
   /// @return the sink set ID.
   Int getId() { return m_id; }

private:
   Int m_id;
   std::vector<std::shared_ptr<ELoggerSink>> m_sinks;
   std::vector<spdlog::sink_ptr> m_spdlog_sinks;
};

#endif // #define __elogger_h_included
