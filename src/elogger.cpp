/*
* Copyright (c) 2009-2019 Brian Waters
* Copyright (c) 2019 Sprint
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in com:e pliance with the License.
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

#include <vector>
#include <unordered_map>
#include <sys/stat.h>

#include "einternal.h"
#include "elogger.h"
#include "epath.h"
#include "estatic.h"
#include "eutil.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/syslog_sink.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

ELoggerError_LogNotFound::ELoggerError_LogNotFound(Int err)
{
   setTextf("Log ID [%d] not found", err);
}

ELoggerError_LogExists::ELoggerError_LogExists(Int err)
{
   setTextf("Log ID [%d] already exists", err);
}

ELoggerError_SinkSetNotFound::ELoggerError_SinkSetNotFound(Int err)
{
   setTextf("Sink set ID [%d] not found", err);
}

ELoggerError_SinkSetExists::ELoggerError_SinkSetExists(Int err)
{
   setTextf("Sink set ID [%d] already exists", err);
}

ELoggerError_SinkSetCreatePath::ELoggerError_SinkSetCreatePath(Int err, cpStr msg)
{
   setTextf( "Error creating directory [%s]", msg );
   appendLastOsError( err );
}

ELoggerError_SinkSetNotDirectory::ELoggerError_SinkSetNotDirectory(cpStr msg)
{
   setText( msg );
}

ELoggerError_SinkSetUnrecognizedSinkType::ELoggerError_SinkSetUnrecognizedSinkType(cpStr msg)
{
   setText( msg );
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ELoggerInit : public EStatic
{
public:
   virtual Int getInitType() { return STATIC_INIT_TYPE_SHARED_OBJECT_MANAGER; }
   Void init(EGetOpt &options)
   {
      ELogger::init(options);
   }
   Void uninit()
   {
      ELogger::uninit();
   }
};

static ELoggerInit _loggerInitializetion;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

EString ELoggerSink::m_defaultpattern = "[%Y-%m-%dT%H:%M:%S.%e] [%^__APPNAME__%$] [%n] [%^%l%$] %v";

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

EString ELogger::m_appname = "ELogger";
std::unordered_map<Int, std::shared_ptr<ELoggerSinkSet>> ELogger::m_sinksets;
std::unordered_map<Int, std::shared_ptr<ELogger>> ELogger::m_logs;
std::map<std::string,std::shared_ptr<ELogger>> ELogger::m_logs_by_name;


ELogger::ELogger(Int logid, cpStr category, Int sinkid)
   : m_logid(logid),
     m_sinkid(sinkid),
     m_category(category),
     m_log(std::make_shared<spdlog::async_logger>(
        m_category.c_str(),
        ELogger::sinkSet(m_sinkid).getSpdlogVector().begin(),
        ELogger::sinkSet(m_sinkid).getSpdlogVector().end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::overrun_oldest))
{
}

Void ELogger::init(EGetOpt &opt)
{
   try
   {
      //
      // set the application name
      //
      cpStr appname = opt.get( SECTION_TOOLS "/" SECTION_LOGGER "/" MEMBER_LOGGER_APPLICATION_NAME, "epctools" );
      ELogger::applicationName( appname );

      //
      // initialize the spdlog global thread pool
      //
      size_t queuesize = opt.get( SECTION_TOOLS "/" SECTION_LOGGER "/" MEMBER_LOGGER_QUEUE_SIZE, 8192 );
      size_t nbrthreads = opt.get( SECTION_TOOLS "/" SECTION_LOGGER "/" MEMBER_LOGGER_NUMBER_THREADS, 1 );

      spdlog::init_thread_pool(queuesize, nbrthreads);

      //
      // create the sink sets
      //
      EString pth;
      opt.setPrefix( SECTION_TOOLS "/" SECTION_LOGGER "/" MEMBER_LOGGER_SINK_SETS );
      Int sscnt = opt.getCount( "" );
      for (int i=0; i<sscnt; i++)
      {
         Int sinkid = opt.get( i, "", MEMBER_LOGGER_SINK_ID, -1 );
         if (sinkid == -1)
            throw ELoggerError_SinkSetSinkIdNotSpecified();

         ELogger::createSinkSet( sinkid );

         pth.format( "%d/" MEMBER_LOGGER_SINKS, i );
         Int scnt = opt.getCount( pth );
         for (Int j=0; j<scnt; j++)
         {
            EString sinktype = opt.get( j, pth, MEMBER_LOGGER_SINK_TYPE, "" );
            EString pattern = opt.get( j, pth, MEMBER_LOGGER_PATTERN, ELoggerSink::getDefaultPattern().c_str() );
            LogLevel loglevel =
               (LogLevel)spdlog::level::from_str( opt.get( j, pth, MEMBER_LOGGER_LOG_LEVEL, "trace" ) );
            
            pattern.replaceAll( "__APPNAME__", 11, ELogger::applicationName(), ELogger::applicationName().size());

            if ( !*sinktype )
               throw ELoggerError_SinkSetSinkTypeNotSpecified();

            if ( sinktype == "basic_file" )
            {
               EString filename = opt.get( j, pth, MEMBER_LOGGER_FILE_NAME, "" );
               Bool truncate = opt.get( j, pth, MEMBER_LOGGER_FILE_TRUNCATE, false );

               verifyPath( filename );

               std::shared_ptr<ELoggerSink> sp = std::make_shared<ELoggerSinkBasicFile>(
                  loglevel, pattern, filename, truncate );

               ELogger::sinkSet(sinkid).addSink( sp );
            }
            else if ( sinktype == "rotating_file" )
            {
               EString filename = opt.get( j, pth, MEMBER_LOGGER_FILE_NAME, "" );
               size_t maxsizemb = opt.get( j, pth, MEMBER_LOGGER_MAX_SIZE, (size_t)100 );
               size_t maxfiles = opt.get( j, pth, MEMBER_LOGGER_MAX_FILES, 2 );
               Bool rotateonopen = opt.get( j, pth, MEMBER_LOGGER_FILE_ROTATE_ON_OPEN, false );

               verifyPath( filename );

               std::shared_ptr<ELoggerSink> sp = std::make_shared<ELoggerSinkRotatingFile>(
                  loglevel, pattern, filename, maxsizemb, maxfiles, rotateonopen );

               ELogger::sinkSet(sinkid).addSink( sp );
            }
            else if ( sinktype == "daily_file" )
            {
               EString filename = opt.get( j, pth, MEMBER_LOGGER_FILE_NAME, "" );
               Bool truncate = opt.get( j, pth, MEMBER_LOGGER_FILE_TRUNCATE, false );
               Int hr = opt.get( j, pth, MEMBER_LOGGER_ROLLOVER_HOUR, 0 );
               Int mi = opt.get( j, pth, MEMBER_LOGGER_ROLLOVER_MINUTE, 0 );

               verifyPath( filename );

               std::shared_ptr<ELoggerSink> sp = std::make_shared<ELoggerSinkDailyFile>(
                  loglevel, pattern, filename, truncate, hr, mi );

               ELogger::sinkSet(sinkid).addSink( sp );
            }
            else if ( sinktype == "stdout" )
            {
               std::shared_ptr<ELoggerSink> sp = std::make_shared<ELoggerSinkStdout>(
                  loglevel, pattern );

               ELogger::sinkSet(sinkid).addSink( sp );
            }
            else if ( sinktype == "stderr" )
            {
               std::shared_ptr<ELoggerSink> sp = std::make_shared<ELoggerSinkStderr>(
                  loglevel, pattern );

               ELogger::sinkSet(sinkid).addSink( sp );
            }
            else if ( sinktype == "syslog" )
            {
               std::shared_ptr<ELoggerSink> sp = std::make_shared<ELoggerSinkSyslog>(
                  loglevel, pattern );

               ELogger::sinkSet(sinkid).addSink( sp );
            }
            else
            {
               throw ELoggerError_SinkSetUnrecognizedSinkType( sinktype );
            }
         }
      }
      opt.setPrefix( "" );

      //
      // create the logs
      //
      opt.setPrefix( SECTION_TOOLS "/" SECTION_LOGGER "/" MEMBER_LOGGER_LOGS );
      Int logcnt = opt.getCount( "" );
      for (int i=0; i<logcnt; i++)
      {
         Int logid = opt.get( i, "", MEMBER_LOGGER_LOG_ID, -1 );
         Int sinkid = opt.get( i, "", MEMBER_LOGGER_SINK_ID, -1 );
         cpStr category = opt.get( i, "", MEMBER_LOGGER_CATEGORY, "" );
         LogLevel loglevel =
            (LogLevel)spdlog::level::from_str( opt.get( i, "", MEMBER_LOGGER_LOG_LEVEL, "trace" ) );

         if (logid == -1)
            throw ELoggerError_SinkSetLogIdNotSpecified();
         if (sinkid == -1)
            throw ELoggerError_SinkSetSinkIdNotSpecified();
         if (!*category)
            throw ELoggerError_SinkSetCategoryNotSpecified();

         ELogger::createLog( logid, category, sinkid );
         ELogger::log(logid).setLogLevel( loglevel );
      }
      opt.setPrefix( "" );
   }
   catch(...)
   {
      opt.setPrefix( "" );
      throw;
   }   
}

Void ELogger::uninit()
{
   spdlog::shutdown();
}

ELogger &ELogger::createLog(Int logid, cpStr category, Int sinkid)
{
   if (m_logs.find(logid) != m_logs.end())
      throw ELoggerError_LogExists(logid);
   
   if (m_sinksets.find(sinkid) == m_sinksets.end())
      throw ELoggerError_SinkSetNotFound(sinkid);
   
   auto lsp = std::make_shared<ELogger>(logid, category, sinkid);

   m_logs[logid] = lsp;
   m_logs_by_name[lsp->get_name()] = lsp;

   return log(logid);
}

ELoggerSinkSet &ELogger::createSinkSet(Int sinkid)
{
   if (m_sinksets.find(sinkid) != m_sinksets.end())
      throw ELoggerError_SinkSetExists(sinkid);

   auto sssp = std::make_shared<ELoggerSinkSet>(sinkid);

   m_sinksets[sinkid] = sssp;

   return sinkSet(sinkid);
}

Void ELogger::verifyPath(cpStr filename)
{
   struct stat st;
   EString dir;
   EPath::getDirectoryName( filename, dir );
   std::vector<EString> dirs = EUtility::split( dir, "/" );

   dir = "";
   for (auto d : dirs)
   {
      dir = EPath::combine( dir, d );
      if ( stat(dir, &st) != 0 )
      {
         if ( mkdir(dir,0777) && errno != EEXIST )
            throw ELoggerError_SinkSetCreatePath( errno, dir );
      }
      else if ( !S_ISDIR(st.st_mode) )
      {
         throw ELoggerError_SinkSetNotDirectory( dir );
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ELoggerSinkSyslog::ELoggerSinkSyslog( ELogger::LogLevel loglevel, cpStr pattern )
   : ELoggerSink( ELoggerSink::eSyslog, loglevel, pattern )
{
   spdlog::sink_ptr sp = std::make_shared<spdlog::sinks::syslog_sink_mt>(
      "", LOG_USER, 0, true );
   sp->set_level( (spdlog::level::level_enum)loglevel );
   setSinkPtr( sp );
   sp->set_pattern( getPattern() );
}

ELoggerSinkStdout::ELoggerSinkStdout( ELogger::LogLevel loglevel, cpStr pattern )
   : ELoggerSink( ELoggerSink::eStdout, loglevel, pattern )
{
   spdlog::sink_ptr sp = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
   sp->set_level( (spdlog::level::level_enum)loglevel );
   setSinkPtr( sp );
   sp->set_pattern( getPattern() );
}

ELoggerSinkStderr::ELoggerSinkStderr( ELogger::LogLevel loglevel, cpStr pattern )
   : ELoggerSink( ELoggerSink::eStderr, loglevel, pattern )
{
   spdlog::sink_ptr sp = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
   sp->set_level( (spdlog::level::level_enum)loglevel );
   setSinkPtr( sp );
   sp->set_pattern( getPattern() );
}

ELoggerSinkBasicFile::ELoggerSinkBasicFile( ELogger::LogLevel loglevel, cpStr pattern,
      cpStr filename, Bool truncate )
   : ELoggerSink( ELoggerSink::eBasicFile, loglevel, pattern ),
     m_filename( filename ),
     m_truncate( truncate )
{
   spdlog::sink_ptr sp = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
      m_filename, m_truncate );
   sp->set_level( (spdlog::level::level_enum)loglevel );
   setSinkPtr( sp );
   sp->set_pattern( getPattern() );
}

ELoggerSinkDailyFile::ELoggerSinkDailyFile( ELogger::LogLevel loglevel, cpStr pattern,
      cpStr filename, Bool truncate, Int rolloverhour, Int rolloverminute )
   : ELoggerSink( ELoggerSink::eDailyFile, loglevel, pattern ),
     m_filename( filename ),
     m_truncate( truncate ),
     m_rolloverhour( rolloverhour ),
     m_rolloverminute( rolloverminute )
{
   spdlog::sink_ptr sp = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
      m_filename, m_rolloverhour, m_rolloverminute, m_truncate );
   sp->set_level( (spdlog::level::level_enum)loglevel );
   setSinkPtr( sp );
   sp->set_pattern( getPattern() );
}

ELoggerSinkRotatingFile::ELoggerSinkRotatingFile( ELogger::LogLevel loglevel, cpStr pattern,
      cpStr filename, size_t maxsizemb, size_t maxfiles, Bool rotateonopen )
   : ELoggerSink( ELoggerSink::eRotatingFile, loglevel, pattern ),
     m_filename( filename ),
     m_maxsizemb( maxsizemb ),
     m_maxfiles( maxfiles ),
     m_rotateonopen( rotateonopen )
{
   spdlog::sink_ptr sp = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      m_filename, m_maxsizemb * 1048576, m_maxfiles, m_rotateonopen );
   sp->set_level( (spdlog::level::level_enum)loglevel );
   setSinkPtr( sp );
   sp->set_pattern( getPattern() );
}
