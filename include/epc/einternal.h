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

class EpcTools
{
public:
   static Void Initialize(EGetOpt &options);
   static Void UnInitialize();

   static Int getInternalLogId() { return m_internalLogId; }
   static Void setInternalLogId(Int logid) { m_internalLogId = logid; }

   static Int getApplicationId() { return m_appid; }
   static Void setApplicationId(Int appid) { m_appid = appid; }

   static Bool isPublicEnabled() { return m_public; }
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

#define SECTION_TOOLS "EpcTools"
#define SECTION_SYNCH_OBJS "SynchronizationObjects"
#define SECTION_PUBLIC_QUEUE "PublicQueue"
#define SECTION_LOGGER "Logger"
#define SECTION_LOGGER_OPTIONS "LoggerOptions"

#define MEMBER_ENABLE_PUBLIC_OBJECTS "EnablePublicObjects"
#define MEMBER_NUMBER_SEMAPHORES "NumberSemaphores"
#define MEMBER_NUMBER_MUTEXES "NumberMutexes"
#define MEMBER_QUEUE_ID "QueueID"
#define MEMBER_MESSAGE_SIZE "MessageSize"
#define MEMBER_QUEUE_SIZE "QueueSize"
#define MEMBER_ALLOW_MULTIPLE_READERS "AllowMultipleReaders"
#define MEMBER_ALLOW_MULTIPLE_WRITERS "AllowMultipleWriters"
#define MEMBER_DEBUG "Debug"
#define MEMBER_WRITE_TO_FILE "WriteToFile"
#define MEMBER_QUEUE_MODE "QueueMode"
#define MEMBER_LOG_ID "LogID"
#define MEMBER_SEGMENTS "Segments"
#define MEMBER_LINESPERSEGMENT "LinesPerSegment"
#define MEMBER_FILENAMEMASK "FileNameMask"
#define MEMBER_LOGTYPE "LogType"
#define MEMBER_DEFAULTLOGMASK "DefaultLogMask"
#define MEMBER_INTERNALLOG "InternalLog"

#endif // #define __einternal_h_included
