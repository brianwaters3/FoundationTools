/*
* Copyright (c) 2009-2019 Brian Waters
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

#ifndef __FTINTERNAL_H_INCLUDED
#define __FTINTERNAL_H_INCLUDED

#include "ftbase.h"
#include "ftgetopt.h"
#include "ftstatic.h"
#include "ftlogger.h"

class FoundationTools
{
public:
   static Void Initialize(FTGetOpt &options);
   static Void UnInitialize();

   static Int getInternalLogId() { return m_internalLogId; }
   static Void setInternalLogId(Int logid) { m_internalLogId = logid; }

   static Int getApplicationId() { return m_appid; }
   static Void setApplicationId(Int appid) { m_appid = appid; }

private:
   static Int m_internalLogId;
   static Int m_appid;
};

#define FTLOG_MUTEX ((ULongLong)0x0000000000000001)
#define FTLOG_SEMAPHORE ((ULongLong)0x0000000000000002)
#define FTLOG_SEMNOTICE ((ULongLong)0x0000000000000004)
#define FTLOG_SHAREDMEMORY ((ULongLong)0x0000000000000008)
#define FTLOG_SYNCHOBJECTS ((ULongLong)0x0000000000000010)

#define FTLOGFUNC(f) static cpStr __funcname__ = #f
#define FTLOG(groupid, severity, format...) FTLogger::log(FoundationTools::getInternalLogId(), groupid, severity, __funcname__, format)
#define FTLOGINFO(groupid, format...) FTLogger::logInfo(FoundationTools::getInternalLogId(), groupid, __funcname__, format)
#define FTLOGWARN(groupid, format...) FTLogger::logWarning(FoundationTools::getInternalLogId(), groupid, __funcname__, format)
#define FTLOGERROR(groupid, format...) FTLogger::logError(FoundationTools::getInternalLogId(), groupid, __funcname__, format)

#define SECTION_TOOLS "FoundationTools"
#define SECTION_SYNCH_OBJS "SynchronizationObjects"
#define SECTION_PUBLIC_QUEUE "PublicQueue"
#define SECTION_LOGGER "FTLogger"
#define SECTION_LOGGER_OPTIONS "FTLoggerOptions"

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
#define MEMBER_FTINTERNALLOG "FTInternalLog"

#endif // #define __FTINTERNAL_H_INCLUDED
