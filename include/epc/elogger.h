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

#ifndef __elogger_h_included
#define __elogger_h_included

#include "ebase.h"
#include "estring.h"
#include "etime.h"
#include "eshmem.h"
#include "eqpub.h"
#include "egetopt.h"
#include "estatic.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR(ELoggerError_MaximumNumberOfLogsDefined);
DECLARE_ERROR_ADVANCED2(ELoggerError_AlreadyExists);
DECLARE_ERROR_ADVANCED2(ELoggerError_LogNotFound);
DECLARE_ERROR_ADVANCED3(ELoggerError_UnableToOpenLogFile);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FoundationTools;
namespace Velocity
{
class BasicApplication;
class LogicModuleInstance;
}; // namespace Velocity

#define ELOGGER_BUFFER_SIZE 8192
#define ELOGGER_MAX_LOGS 128

#define ELOG_RECORDID 1

//class ELogger : public EStatic
class ELogger
{
   friend class FoundationTools;
   friend class Velocity::BasicApplication;
   friend class Velocity::LogicModuleInstance;

public:
   enum Severity
   {
      Error = 1,
      Warning = 2,
      Info = 3,
      Debug = 4
   };
   enum LogType
   {
      ltFile = 1,
      ltSysLog = 2
   };

protected:
   typedef struct
   {
      Int s_logid;
      longinteger_t s_mask;
      Int s_maxsegs;
      Int s_linesperseg;
      Char s_filenamemask[EPC_FILENAME_MAX];
      LogType s_logtype;
   } eloggerentry_t;

   typedef struct
   {
      Bool s_sharedmem;
      Bool s_initialized;
      eloggerentry_t s_logs[ELOGGER_MAX_LOGS];
   } eloggerctrl_t;

   class eloggerloghandle_t
   {
   public:
      eloggerloghandle_t() : s_mutex(false) {}
      ~eloggerloghandle_t() {}

      EMutexPrivate s_mutex;
      Int s_fh;
      Int s_currseg;
      Int s_linecnt;
      Char s_buffer[ELOGGER_BUFFER_SIZE];
   };

public:
   class ELoggerQueueMessage : public EQueueMessage
   {
   public:
      ELoggerQueueMessage()
      {
         setMsgType(ELOG_RECORDID);

         m_logid = 0;
         m_groupid = 0;
         m_severity = Info;
         m_sequence = 0;
         memset(m_func, 0, sizeof(m_func));
         memset(m_msg, 0, sizeof(m_msg));
      }

      ~ELoggerQueueMessage()
      {
      }

      virtual Void getLength(ULong &length)
      {
         EQueueMessage::getLength(length);
         elementLength(m_time, length);
         elementLength(m_logid, length);
         elementLength(m_groupid, length);
         elementLength((Long)m_severity, length);
         elementLength(m_sequence, length);
         elementLength(m_func, length);
         elementLength(m_msg, length);
      }
      virtual Void serialize(pVoid pBuffer, ULong &nOffset)
      {
         EQueueMessage::serialize(pBuffer, nOffset);
         pack(m_time, pBuffer, nOffset);
         pack(m_logid, pBuffer, nOffset);
         pack(m_groupid, pBuffer, nOffset);
         pack((Long)m_severity, pBuffer, nOffset);
         pack(m_sequence, pBuffer, nOffset);
         pack(m_func, pBuffer, nOffset);
         pack(m_msg, pBuffer, nOffset);
      }
      virtual Void unserialize(pVoid pBuffer, ULong &nOffset)
      {
         Long severity;

         EQueueMessage::unserialize(pBuffer, nOffset);
         unpack(m_time, pBuffer, nOffset);
         unpack(m_logid, pBuffer, nOffset);
         unpack(m_groupid, pBuffer, nOffset);
         unpack(severity, pBuffer, nOffset);
         m_severity = (ELogger::Severity)severity;
         unpack(m_sequence, pBuffer, nOffset);
         unpack(m_func, pBuffer, nOffset);
         unpack(m_msg, pBuffer, nOffset);
      }

      Void set(Int logid, ULongLong groupid, ELogger::Severity esev,
               ETime &t, Long seq, cpStr func, cpStr msg)
      {
         m_time = t;
         m_logid = logid;
         m_groupid = groupid;
         m_severity = esev;
         m_sequence = seq;
         epc_strcpy_s(m_func, sizeof(m_func), func);
         epc_strcpy_s(m_msg, sizeof(m_msg), msg);
      }

      ETime &getTime() { return m_time; }
      Int getLogId() { return m_logid; }
      ULongLong getGroupId() { return m_groupid; }
      ELogger::Severity getSeverity() { return m_severity; }
      Long getSequence() { return m_sequence; }
      cpStr getFunction() { return m_func; }
      cpStr getMessage() { return m_msg; }

   private:
      ETime m_time;
      Long m_logid;
      ULongLong m_groupid;
      ELogger::Severity m_severity;
      Long m_sequence;
      Char m_func[EPC_FILENAME_MAX];
      Char m_msg[2048];
   };

   static Bool isGroupMaskEnabled(Int logid, ULongLong groupMask);
   static Void enableGroupMask(Int logid, ULongLong groupMask);
   static Void disableGroupMask(Int logid, ULongLong groupMask);

   static Void setGroupMask(Int logid, ULongLong newGroupMask);
   static ULongLong getGroupMask(Int logid);

   static Void log(ELoggerQueueMessage &msg);
   static Void logInfo(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...);
   static Void logError(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...);
   static Void logWarning(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...);
   static Void logDebug(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...);
   static Void log(Int logid, ULongLong groupid, Severity esev, cpStr pszFunc, cpStr pszText, ...);

   static Bool isLogIdValid(Int logid);

   static Void setLoggerPtr(ELogger *pThis);
   ELogger *getLoggerPtr() { return m_pThis; }

   ELogger();
   ~ELogger();

   //virtual Int getInitType() { return STATIC_INIT_TYPE_PRIORITY; }
   Void init(EGetOpt &options);
   Void uninit();

   cpStr getSeverityText(Severity eSeverity) { return m_pszSeverity[eSeverity]; }

   class ELoggerQueue : public EQueuePublic
   {
   public:
      ELoggerQueueMessage &getMsg()
      {
         return m_msg;
      }

   protected:
      EQueueMessage *allocMessage(Long msgType)
      {
         if (msgType == 1)
            return &m_msg;
         return NULL;
      }

   private:
      ELoggerQueueMessage m_msg;
   };

protected:
   static cpStr m_pszSeverity[];
   static ELogger *m_pThis;

   Void addLog(Int logid, ULongLong defaultmask, Int maxsegments, Int linespersegment, cpChar filename, LogType logtype);
   eloggerentry_t *findLog(Int logid, Int *plogofs = NULL);

   static Void log(Int logid, ULongLong groupid, Severity esev, cpStr pszFunc, cpStr pszText, va_list &args);
   static Bool groupEnabled(eloggerentry_t *pLog, ULongLong group)
   {
      return ((pLog->s_mask.quadPart & group) == 0) ? False : True;
   }

private:
   Void setNextSegment(eloggerentry_t *pLog, eloggerloghandle_t &h);
   Void buildFileName(eloggerentry_t *pLog, eloggerloghandle_t &h, EString &s);
   Void verifyHandle(eloggerentry_t *pLog, eloggerloghandle_t &h);
   Void writeFile(eloggerentry_t *pLog, Int logofs, Int logid, ULongLong groupid,
                  Severity esev, ETime &t, Long seq, cpStr pszFunc, cpChar msg);
   Void writeSysLog(eloggerentry_t *pLog, Int logofs, Int logid, ULongLong groupid,
                    Severity esev, ETime &t, Long seq, cpStr pszFunc, cpChar msg);
   Void writeQueue(eloggerentry_t *pLog, Int logofs, Int logid, ULongLong groupid,
                   Severity esev, ETime &t, Long seq, cpStr pszFunc, cpChar msg);

   Bool m_writetofile;
   eloggerloghandle_t m_handles[ELOGGER_MAX_LOGS];
   ELoggerQueue m_queue;
   ESharedMemory m_sharedmem;
   eloggerctrl_t *m_pCtrl;
};

#endif // #define __elogger_h_included
