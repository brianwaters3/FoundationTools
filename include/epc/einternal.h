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

#ifndef __einternal_h_included
#define __einternal_h_included

#include "ebase.h"
#include "egetopt.h"
#include "estatic.h"
#include "elogger.h"

/// @file
/// @brief EpcTools management.

/// @brief The primary class used to initialize/uninitialize EpcTools
class EpcTools
{
public:
   /// @brief Initialize the EpcTools runtime environment
   /// @param options The configuration options used to initalize EpcTools
   static Void Initialize(EGetOpt &options);
   /// @brief Uninitialize the EpcTools runtime environment
   static Void UnInitialize();

   /// @brief Gets the log ID associated with internal log messages
   /// @return the log ID
   static Int getInternalLogId() { return m_internalLogId; }
   /// @brief Sets the log ID associated with internal log messages
   /// @param logid The new log ID.
   static Void setInternalLogId(Int logid) { m_internalLogId = logid; }

   /// @brief Gets the application ID for this application
   /// @return the application ID
   static Int getApplicationId() { return m_appid; }
   /// @brief Sets the application ID
   /// @param appid The new application ID
   static Void setApplicationId(Int appid) { m_appid = appid; }

   /// @brief Indicates that public features are enabled.
   /// @return True - public features are enabled, False - public features are not enabled
   static Bool isPublicEnabled() { return m_public; }
   /// @brief Indicates that public features are enabled.
   /// @return True - debug is enabled, False - debug is not enabled
   static Bool isDebug() { return m_debug; }

private:
   static Int m_internalLogId;
   static Int m_appid;
   static Bool m_public;
   static Bool m_debug;
};

// #define ELOG_MUTEX ((ULongLong)0x0000000000000001)
// #define ELOG_SEMAPHORE ((ULongLong)0x0000000000000002)
// #define ELOG_SEMNOTICE ((ULongLong)0x0000000000000004)
// #define ELOG_SHAREDMEMORY ((ULongLong)0x0000000000000008)
// #define ELOG_SYNCHOBJECTS ((ULongLong)0x0000000000000010)

// #define ELOGFUNC(f) static cpStr __funcname__ = #f
// #define ELOG(groupid, severity, format...) ELogger::log(FoundationTools::getInternalLogId(), groupid, severity, __funcname__, format)
// #define ELOGINFO(groupid, format...) ELogger::logInfo(FoundationTools::getInternalLogId(), groupid, __funcname__, format)
// #define ELOGWARN(groupid, format...) ELogger::logWarning(FoundationTools::getInternalLogId(), groupid, __funcname__, format)
// #define ELOGERROR(groupid, format...) ELogger::logError(FoundationTools::getInternalLogId(), groupid, __funcname__, format)

/// @cond DOXYGEN_EXCLUDE
#define SECTION_TOOLS "EpcTools"
#define SECTION_SYNCH_OBJS "SynchronizationObjects"
#define SECTION_PUBLIC_QUEUE "PublicQueue"

#define SECTION_LOGGER "Logger"
#define MEMBER_LOGGER_APPLICATION_NAME "ApplicationName"
#define MEMBER_LOGGER_QUEUE_SIZE "QueueSize"
#define MEMBER_LOGGER_NUMBER_THREADS "NumberThreads"
#define MEMBER_LOGGER_SINK_SETS "SinkSets"
#define MEMBER_LOGGER_SINK_ID "SinkID"
#define MEMBER_LOGGER_SINKS "Sinks"
#define MEMBER_LOGGER_SINK_TYPE "SinkType"
#define MEMBER_LOGGER_LOG_LEVEL "LogLevel"
#define MEMBER_LOGGER_PATTERN "Pattern"
#define MEMBER_LOGGER_FILE_NAME "FileName"
#define MEMBER_LOGGER_FILE_TRUNCATE "Truncate"
#define MEMBER_LOGGER_FILE_ROTATE_ON_OPEN "RotateOnOpen"
#define MEMBER_LOGGER_MAX_SIZE "MaxSizeMB"
#define MEMBER_LOGGER_MAX_FILES "MaxNumberFiles"
#define MEMBER_LOGGER_ROLLOVER_HOUR "RolloverHour"
#define MEMBER_LOGGER_ROLLOVER_MINUTE "RolloverMinute"
#define MEMBER_LOGGER_LOGS "Logs"
#define MEMBER_LOGGER_LOG_ID "LogID"
#define MEMBER_LOGGER_CATEGORY "Category"

#define MEMBER_ENABLE_PUBLIC_OBJECTS "EnablePublicObjects"
#define MEMBER_NUMBER_SEMAPHORES "NumberSemaphores"
#define MEMBER_NUMBER_MUTEXES "NumberMutexes"
#define MEMBER_QUEUE_ID "QueueID"
#define MEMBER_QUEUE_SIZE "QueueSize"
#define MEMBER_MESSAGE_SIZE "MessageSize"
#define MEMBER_ALLOW_MULTIPLE_READERS "AllowMultipleReaders"
#define MEMBER_ALLOW_MULTIPLE_WRITERS "AllowMultipleWriters"
#define MEMBER_DEBUG "Debug"
// #define MEMBER_WRITE_TO_FILE "WriteToFile"
// #define MEMBER_QUEUE_MODE "QueueMode"
// #define MEMBER_LOGGER_ID "LogID"
// #define MEMBER_SEGMENTS "Segments"
// #define MEMBER_LINESPERSEGMENT "LinesPerSegment"
// #define MEMBER_FILENAMEMASK "FileNameMask"
// #define MEMBER_LOGTYPE "LogType"
// #define MEMBER_DEFAULTLOGMASK "DefaultLogMask"
// #define MEMBER_INTERNALLOG "InternalLog"

/// @endcond

#endif // #define __einternal_h_included
