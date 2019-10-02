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

#if 0
#include "ebase.h"
#include "estring.h"
#include "etime.h"
#include "eshmem.h"
#include "eqpub.h"
#include "egetopt.h"
#include "estatic.h"
#endif

#include <vector>
#include <map>
#include <unordered_map>

#ifndef SPDLOG_LEVEL_NAMES
//#define SPDLOG_LEVEL_NAMES { "trace", "debug", "info",  "warning", "error", "critical", "off" };
#define SPDLOG_LEVEL_NAMES { "debug", "info", "startup", "minor", "major", "critical", "off" };
#endif

#define SPDLOG_ENABLE_SYSLOG
#include "spdlog/spdlog.h"
#include "spdlog/async.h"

#include "ebase.h"
#include "eerror.h"
#include "estring.h"
#include "eutil.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

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

class ELoggerSinkSet;

class ELogger
{
   friend class ELoggerInit;

public:
   enum LogLevel
   {
      eDebug      = spdlog::level::trace,
      eInfo       = spdlog::level::debug,
      eStartup    = spdlog::level::info,
      eMinor      = spdlog::level::warn,
      eMajor      = spdlog::level::err,
      eCritical   = spdlog::level::critical,
      eOff        = spdlog::level::off
   };

   ELogger(Int logid, cpStr category, Int sinkid);
   ~ELogger() {}

   static EString &applicationName() { return m_appname; }
   static EString &applicationName(cpStr app) { return m_appname = app; }

   static ELogger &createLog(Int logid, cpStr category, Int sinkid);
   static ELoggerSinkSet &createSinkSet(Int sinkid);

   static ELogger &log(Int logid)
   {
      auto srch = m_logs.find(logid);
      if (srch == m_logs.end())
         throw ELoggerError_LogNotFound(logid);
      return *srch->second;
   }

   static ELoggerSinkSet &sinkSet(Int sinkid)
   {
      if (m_sinksets.find(sinkid) == m_sinksets.end())
         throw ELoggerError_SinkSetNotFound(sinkid);
      
      return *m_sinksets[sinkid];
   }

   template<typename Arg1, typename... Args>
   Void trace( cpStr format, const Arg1 &arg1, const Args &... args) { m_log->trace(format, arg1, args...); }
   template<typename Arg1, typename... Args>
   Void debug( cpStr format, const Arg1 &arg1, const Args &... args) { m_log->debug(format, arg1, args...); }
   template<typename Arg1, typename... Args>
   Void info( cpStr format, const Arg1 &arg1, const Args &... args) { m_log->info(format, arg1, args...); }
   template<typename Arg1, typename... Args>
   Void startup( cpStr format, const Arg1 &arg1, const Args &... args) { m_log->warn(format, arg1, args...); }
   template<typename Arg1, typename... Args>
   Void warn( cpStr format, const Arg1 &arg1, const Args &... args) { m_log->error(format, arg1, args...); }
   template<typename Arg1, typename... Args>
   Void error( cpStr format, const Arg1 &arg1, const Args &... args) { m_log->critical(format, arg1, args...); }

   Void flush() { m_log->flush(); }

   Void setLogLevel( LogLevel lvl ) { m_log->set_level((spdlog::level::level_enum)lvl); }
   LogLevel getLogLevel() { return (LogLevel)m_log->level(); }

   const std::string & get_name() { return m_log->name(); }
   const std::map<std::string,std::shared_ptr<ELogger>> get_loggers() { return m_logs_by_name; }

protected:
   static Void init(EGetOpt &opt);
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

class ELoggerSink
{
public:
   enum SinkType
   {
      eSyslog,
      eStdout,
      eStderr,
      eBasicFile,
      eRotatingFile,
      eDailyFile
   };

   virtual ~ELoggerSink() {}

   SinkType getSinkType() { return m_sinktype; }
   ELogger::LogLevel getLogLevel() { return (ELogger::LogLevel)m_sinkptr->level(); }
   EString &getPattern() { return m_pattern; }

   ELogger::LogLevel setLogLevel( ELogger::LogLevel loglevel )
      { m_sinkptr->set_level( (spdlog::level::level_enum)loglevel ); m_loglevel = loglevel; return getLogLevel(); }
   EString &setPattern( cpStr pattern )
      { m_pattern = pattern; m_sinkptr->set_pattern( m_pattern ); return getPattern(); }

   spdlog::sink_ptr getSinkPtr() { return m_sinkptr; }

   static EString &getDefaultPattern() { return m_defaultpattern; }

protected:
   ELoggerSink( SinkType sinktype, ELogger::LogLevel loglevel, cpStr pattern )
      : m_sinktype( sinktype ),
        m_loglevel( loglevel ),
        m_pattern( pattern )
   {
   }

   spdlog::sink_ptr setSinkPtr( spdlog::sink_ptr &sinkptr ) { return m_sinkptr = sinkptr; }

private:
   ELoggerSink();
   static EString m_defaultpattern;

   SinkType m_sinktype;
   ELogger::LogLevel m_loglevel;
   EString m_pattern;
   spdlog::sink_ptr m_sinkptr;
};

class ELoggerSinkSyslog : public ELoggerSink
{
public:
   ELoggerSinkSyslog( ELogger::LogLevel loglevel, cpStr pattern );
   virtual ~ELoggerSinkSyslog() {}
private:
   ELoggerSinkSyslog();
};

class ELoggerSinkStdout : public ELoggerSink
{
public:
   ELoggerSinkStdout( ELogger::LogLevel loglevel, cpStr pattern );
   virtual ~ELoggerSinkStdout() {}
private:
   ELoggerSinkStdout();
};

class ELoggerSinkStderr : public ELoggerSink
{
public:
   ELoggerSinkStderr( ELogger::LogLevel loglevel, cpStr pattern );
   virtual ~ELoggerSinkStderr() {}
private:
   ELoggerSinkStderr();
};

class ELoggerSinkBasicFile : public ELoggerSink
{
public:
   ELoggerSinkBasicFile( ELogger::LogLevel loglevel, cpStr pattern,
      cpStr filename, Bool truncate );
   virtual ~ELoggerSinkBasicFile() {}

   EString &getFilename() { return m_filename; }
   Bool getTruncate() { return m_truncate; }

private:
   EString m_filename;
   Bool m_truncate;
};

class ELoggerSinkRotatingFile : public ELoggerSink
{
public:
   ELoggerSinkRotatingFile( ELogger::LogLevel loglevel, cpStr pattern,
      cpStr filename, size_t maxsizemb, size_t maxfiles, Bool rotateonopen );
   virtual ~ELoggerSinkRotatingFile() {}

   EString &getFilename() { return m_filename; }
   size_t getMaxSizeMB() { return m_maxsizemb; }
   size_t getMaxFiles() { return m_maxfiles; }
   Bool getRotateOnOpen() { return m_rotateonopen; }

private:
   EString m_filename;
   size_t m_maxsizemb;
   size_t m_maxfiles;
   Bool m_rotateonopen;
};

class ELoggerSinkDailyFile : public ELoggerSink
{
public:
   ELoggerSinkDailyFile( ELogger::LogLevel loglevel, cpStr pattern,
      cpStr filename, Bool truncate, Int rolloverhour, Int rolloverminute );
   virtual ~ELoggerSinkDailyFile() {}

   EString &getFilename() { return m_filename; }
   Bool getTruncate() { return m_truncate; }
   Int getRolloverHour() { return m_rolloverhour; }
   Int getRolloverMinute() { return m_rolloverminute; }

private:
   EString m_filename;
   Bool m_truncate;
   Int m_rolloverhour;
   Int m_rolloverminute;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ELoggerSinkSet
{
   friend ELogger;
public:
   ELoggerSinkSet(Int id=-1) : m_id(id) {}
   ~ELoggerSinkSet() {}

   Void addSink(std::shared_ptr<ELoggerSink> &sink)
   {
      m_sinks.push_back( sink );
      m_spdlog_sinks.push_back( sink->getSinkPtr() );
   }

   std::vector<std::shared_ptr<ELoggerSink>> &getVector() { return m_sinks; }
   std::vector<spdlog::sink_ptr> &getSpdlogVector() { return m_spdlog_sinks; }

   Int setId(Int id) { return m_id = id; }
   Int getId() { return m_id; }

private:
   Int m_id;
   std::vector<std::shared_ptr<ELoggerSink>> m_sinks;
   std::vector<spdlog::sink_ptr> m_spdlog_sinks;
};

#endif // #define __elogger_h_included
