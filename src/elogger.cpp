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

#include <vector>
#include <stdarg.h>

#include "elogger.h"
#include "eatomic.h"
#include "esynch.h"
#include "esynch2.h"
#include "einternal.h"

#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ELoggerError_AlreadyExists::ELoggerError_AlreadyExists(Int err)
{
   setSevere();
   setTextf("Log entry already exists for log [%d].", err);
}

ELoggerError_LogNotFound::ELoggerError_LogNotFound(Int err)
{
   setSevere();
   setTextf("Log [%d] not found.", err);
}

ELoggerError_UnableToOpenLogFile::ELoggerError_UnableToOpenLogFile(Int err, cpStr msg)
{
   setSevere();
   setTextf("Error opening [%s] - ", msg);
   appendLastOsError(err);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

cpStr ELogger::m_pszSeverity[] = {"Undefined", "Error", "Warning", "Info", "Debug"};

class ELoggerControl : public EStatic
{
public:
   ELoggerControl() {}

   ~ELoggerControl() {}

   virtual Void init(EGetOpt &opt)
   {
      m_logCtrl = new ELogger();
      m_logCtrl->init(opt);
   }

   virtual Void uninit()
   {
      m_logCtrl->uninit();
      delete m_logCtrl;
   }

   virtual Int getInitType() { return STATIC_INIT_TYPE_PRIORITY; }

private:
   ELogger *m_logCtrl;
};

ELoggerControl _logCtrl;
ELogger *ELogger::m_pThis = NULL;

Void ELogger::setLoggerPtr(ELogger *pThis)
{
   //_logCtrl.m_pThis = pThis;
}

ELogger::ELogger()
{
   m_pThis = this;

   m_writetofile = False;
   for (Int ofs = 0; ofs < ELOGGER_MAX_LOGS; ofs++)
   {
      m_handles[ofs].s_fh = -1;
      m_handles[ofs].s_currseg = -1;
      m_handles[ofs].s_linecnt = 0;
   }
}

ELogger::~ELogger()
{
   closelog(); // close syslog
}

Void ELogger::init(EGetOpt &options)
{
   options.setPrefix(SECTION_TOOLS "/" SECTION_LOGGER_OPTIONS);
   Bool bWriteToFile = options.get(MEMBER_WRITE_TO_FILE, false);
   Int nQueueID = options.get(MEMBER_QUEUE_ID, 0);
   EString s;
   EQueueBase::Mode mode;

   s = options.get(MEMBER_QUEUE_MODE, "WriteOnly");
   s.tolower();

   if (s == "readonly")
      mode = EQueueBase::ReadOnly;
   else if (s == "writeonly")
      mode = EQueueBase::WriteOnly;
   else
      mode = EQueueBase::ReadWrite;

   options.setPrefix("");

   ////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////

   m_writetofile = bWriteToFile;

   if (!m_writetofile)
      m_queue.init(nQueueID, mode);

   if (options.get(SECTION_TOOLS "/" MEMBER_ENABLE_PUBLIC_OBJECTS, false))
   {
      m_sharedmem.init("ELoggerControlBlock", 1, sizeof(eloggerctrl_t));

      m_pCtrl = (eloggerctrl_t *)m_sharedmem.getDataPtr();
      m_pCtrl->s_sharedmem = True;
   }
   else
   {
      m_pCtrl = new eloggerctrl_t();
      memset(m_pCtrl, 0, sizeof(*m_pCtrl));
      m_pCtrl->s_sharedmem = False;
   }

   if (!m_pCtrl->s_initialized)
   {
      for (int ofs = 0; ofs < ELOGGER_MAX_LOGS; ofs++)
      {
         m_pCtrl->s_logs[ofs].s_logid = -1;
         m_pCtrl->s_logs[ofs].s_mask.quadPart = 0;
         m_pCtrl->s_logs[ofs].s_maxsegs = 1;
         m_pCtrl->s_logs[ofs].s_linesperseg = 100000;
         m_pCtrl->s_logs[ofs].s_filenamemask[0] = '\0';
      }

      m_pCtrl->s_initialized = True;
   }

   ////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////

   UInt cnt;

   options.setPrefix(SECTION_TOOLS);
   cnt = options.getCount(SECTION_LOGGER);

   for (UInt idx = 0; idx < cnt; idx++)
   {
      Int logid;
      ULongLong defaultmask;
      Int maxsegments;
      Int linespersegment;
      EString filenamemask;
      EString s;
      LogType logtype = ELogger::ltFile;

      logid = options.get(idx, SECTION_LOGGER, MEMBER_LOG_ID, -1);
      maxsegments = options.get(idx, SECTION_LOGGER, MEMBER_SEGMENTS, -1);
      linespersegment = options.get(idx, SECTION_LOGGER, MEMBER_LINESPERSEGMENT, -1);
      filenamemask = options.get(idx, SECTION_LOGGER, MEMBER_FILENAMEMASK, "./elog_%A_%S.log");
      s = options.get(idx, SECTION_LOGGER, MEMBER_LOGTYPE, "File");
      logtype = (s.tolower() == "syslog") ? ltSysLog : ltFile;
      s = options.get(idx, SECTION_LOGGER, MEMBER_DEFAULTLOGMASK, "0x0000000000000000");
      defaultmask = strtoull(s.c_str(), NULL, 0);

      addLog(logid, defaultmask, maxsegments, linespersegment, filenamemask, logtype);

      if (options.get(idx, SECTION_LOGGER, MEMBER_INTERNALLOG, false))
         EpcTools::setInternalLogId(logid);
   }
}

Void ELogger::uninit()
{
   if (m_pCtrl && m_pCtrl->s_initialized)
   {
      for (Int ofs = 0; ofs < ELOGGER_MAX_LOGS; ofs++)
      {
         if (m_handles[ofs].s_fh != -1)
         {
            close(m_handles[ofs].s_fh);
            m_handles[ofs].s_fh = -1;
            m_handles[ofs].s_linecnt = 0;
            m_handles[ofs].s_currseg = -1;
            m_handles[ofs].s_mutex.destroy();
         }
      }

      if (!m_pCtrl->s_sharedmem)
      {
         delete m_pCtrl;
         m_pCtrl = NULL;
      }      
   }

   m_queue.destroy();
}

Void ELogger::addLog(Int logid, ULongLong defaultmask, Int maxsegments, Int linespersegment, cpChar filenamemask, LogType logtype)
{
   if (findLog(logid) != NULL)
      return;

   Int ofs;
   for (ofs = 0; ofs < ELOGGER_MAX_LOGS && m_pCtrl->s_logs[ofs].s_logid != -1; ofs++)
      ;

   if (ofs == ELOGGER_MAX_LOGS)
      throw ELoggerError_MaximumNumberOfLogsDefined();

   eloggerentry_t *pLog = &m_pCtrl->s_logs[ofs];

   pLog->s_logid = logid;
   pLog->s_mask.quadPart = (LongLong)defaultmask;
   pLog->s_maxsegs = maxsegments;
   pLog->s_linesperseg = linespersegment;
   epc_strcpy_s(pLog->s_filenamemask, sizeof(pLog->s_filenamemask), filenamemask);
   pLog->s_logtype = logtype;

   m_handles[ofs].s_currseg = 0;
   m_handles[ofs].s_mutex.init();
}

Bool ELogger::isGroupMaskEnabled(Int logid, ULongLong groupMask)
{
   eloggerentry_t *pLog = m_pThis->findLog(logid);
   if (pLog == NULL)
      throw ELoggerError_LogNotFound(logid);

   return groupEnabled(pLog, groupMask);
}

Void ELogger::enableGroupMask(Int logid, ULongLong groupMask)
{
   eloggerentry_t *pLog = m_pThis->findLog(logid);
   if (pLog == NULL)
      throw ELoggerError_LogNotFound(logid);

   longinteger_t gm;
   gm.quadPart = (LongLong)groupMask;

   atomic_or(pLog->s_mask.li.lowPart, gm.li.lowPart);
   atomic_or(pLog->s_mask.li.highPart, gm.li.highPart);
}

Void ELogger::disableGroupMask(Int logid, ULongLong groupMask)
{
   eloggerentry_t *pLog = m_pThis->findLog(logid);
   if (pLog == NULL)
      throw ELoggerError_LogNotFound(logid);

   longinteger_t gm;
   gm.quadPart = (LongLong)groupMask;

   atomic_and(pLog->s_mask.li.lowPart, ~gm.li.lowPart);
   atomic_and(pLog->s_mask.li.highPart, ~gm.li.highPart);
}

Void ELogger::setGroupMask(Int logid, ULongLong groupMask)
{
   eloggerentry_t *pLog = m_pThis->findLog(logid);
   if (pLog == NULL)
      throw ELoggerError_LogNotFound(logid);

   pLog->s_mask.quadPart = (LongLong)groupMask;
}

ULongLong ELogger::getGroupMask(Int logid)
{
   eloggerentry_t *pLog = m_pThis->findLog(logid);
   if (pLog == NULL)
      throw ELoggerError_LogNotFound(logid);

   return (ULongLong)pLog->s_mask.quadPart;
}

Void ELogger::logDebug(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...)
{
   if (logid == -1)
      return;

   va_list args;
   va_start(args, pszText);
   log(logid, groupid, ELogger::Debug, pszFunc, pszText, args);
   va_end(args);
}

Void ELogger::logInfo(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...)
{
   if (logid == -1)
      return;

   va_list args;
   va_start(args, pszText);
   log(logid, groupid, ELogger::Info, pszFunc, pszText, args);
   va_end(args);
}

Void ELogger::logWarning(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...)
{
   if (logid == -1)
      return;

   va_list args;
   va_start(args, pszText);
   log(logid, groupid, ELogger::Warning, pszFunc, pszText, args);
   va_end(args);
}

Void ELogger::logError(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...)
{
   if (logid == -1)
      return;

   va_list args;
   va_start(args, pszText);
   log(logid, groupid, ELogger::Error, pszFunc, pszText, args);
   va_end(args);
}

Void ELogger::log(Int logid, ULongLong groupid, Severity esev, cpStr pszFunc, cpStr pszText, ...)
{
   if (logid == -1)
      return;

   va_list args;
   va_start(args, pszText);
   log(logid, groupid, esev, pszFunc, pszText, args);
   va_end(args);
}

Void ELogger::log(Int logid, ULongLong groupid, Severity esev, cpStr pszFunc, cpStr pszText, va_list &args)
{
   if (logid == -1)
      return;

   Int logofs = -1;
   eloggerentry_t *pLog = m_pThis->findLog(logid, &logofs);
   if (pLog == NULL)
      throw ELoggerError_LogNotFound(logid);

   if (groupEnabled(pLog, groupid) || esev == ELogger::Error)
   {
      Char szBuff[ELOGGER_BUFFER_SIZE];
      epc_vsnprintf_s(szBuff, sizeof(szBuff), pszText, args);
      ETime t;

      if (m_pThis->m_writetofile)
      {
         if (pLog->s_logtype == ltSysLog)
            m_pThis->writeSysLog(pLog, logofs, logid, groupid, esev, t,
                                 ESynchObjects::getSynchObjCtrlPtr()->incSequence(), pszFunc, szBuff);
         else
            m_pThis->writeFile(pLog, logofs, logid, groupid, esev, t,
                               ESynchObjects::getSynchObjCtrlPtr()->incSequence(), pszFunc, szBuff);
      }
      else
         m_pThis->writeQueue(pLog, logofs, logid, groupid, esev, t,
                             ESynchObjects::getSynchObjCtrlPtr()->incSequence(), pszFunc, szBuff);
   }
}

Bool ELogger::isLogIdValid(Int logid)
{
   return m_pThis->findLog(logid) == NULL ? False : True;
}

Void ELogger::buildFileName(eloggerentry_t *pLog, eloggerloghandle_t &h, EString &s)
{
   Char mask[EPC_FILENAME_MAX];
   Bool inPercent = False;
   Int val[2] = {0, 0};
   Long valofs = 0;
   Int mOfs = 0;
   Int fOfs = 0;

   for (; pLog->s_filenamemask[fOfs]; fOfs++, mOfs++)
   {
      if (inPercent)
      {
         if (isdigit(pLog->s_filenamemask[fOfs]))
            mask[mOfs] = pLog->s_filenamemask[fOfs];
         else if (pLog->s_filenamemask[fOfs] == 'A')
         {
            mask[mOfs++] = 'l';
            mask[mOfs] = 'd';
            val[valofs++] = EpcTools::getApplicationId();
            inPercent = False;
         }
         else if (pLog->s_filenamemask[fOfs] == 'S')
         {
            mask[mOfs++] = 'l';
            mask[mOfs] = 'd';
            val[valofs++] = h.s_currseg + 1;
            inPercent = False;
         }
      }
      else
      {
         if (pLog->s_filenamemask[fOfs] == '%')
            inPercent = True;
         mask[mOfs] = pLog->s_filenamemask[fOfs];
      }
   }
   mask[mOfs] = '\0';

   s.format(mask, val[0], val[1]);
}

Void ELogger::setNextSegment(eloggerentry_t *pLog, eloggerloghandle_t &h)
{
   Int lastSeg = 0;
   time_t lastTime = 0;
   EString s;
   struct stat st;
   for (h.s_currseg = 0; h.s_currseg < pLog->s_maxsegs; h.s_currseg++)
   {
      buildFileName(pLog, h, s);
      if (stat(s, &st) == 0 && st.st_mtime >= lastTime)
      {
         lastSeg = h.s_currseg;
         lastTime = st.st_mtime;
      }
   }

   if (++lastSeg >= pLog->s_maxsegs)
      lastSeg = 0;

   h.s_currseg = lastSeg;
}

Void ELogger::verifyHandle(eloggerentry_t *pLog, eloggerloghandle_t &h)
{
   if (h.s_fh == -1) // need to initialize
      setNextSegment(pLog, h);

   if (h.s_linecnt >= pLog->s_linesperseg && pLog->s_linesperseg != -1)
   {
      if (h.s_fh != -1)
      {
         close(h.s_fh);
         h.s_fh = -1;
      }
      h.s_linecnt = 0;
      h.s_currseg++;
      if (h.s_currseg >= pLog->s_maxsegs)
         h.s_currseg = 0;
   }

   if (h.s_fh == -1)
   {
      Int oflag, pmode;
      EString s;

      buildFileName(pLog, h, s);

      oflag = O_CREAT | O_TRUNC | O_RDWR;
      pmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
      h.s_fh = open(s, oflag, pmode);
      if (h.s_fh == -1)
         throw ELoggerError_UnableToOpenLogFile(errno, s);
   }
}

Void ELogger::writeQueue(eloggerentry_t *pLog, Int logofs, Int logid,
                          ULongLong groupid, Severity esev, ETime &t, Long seq, cpStr pszFunc, cpChar msg)
{
   ELoggerQueueMessage qmsg;

   qmsg.setMsgType(1);
   qmsg.set(logid, groupid, esev, t, seq, pszFunc, msg);

   m_queue.push(qmsg);
}

Void ELogger::writeFile(eloggerentry_t *pLog, Int logofs, Int logid,
                         ULongLong groupid, Severity sev, ETime &t, Long seq, cpStr pszFunc, cpChar msg)
{
   longinteger_t gm;
   //Char buffer[4096];

   EMutexLock l(m_handles[logofs].s_mutex);

   verifyHandle(pLog, m_handles[logofs]);

   t.Format(m_handles[logofs].s_buffer, sizeof(m_handles[logofs].s_buffer), "%F %H:%M:%S.%0", True);
   gm.quadPart = (LongLong)groupid;

   epc_sprintf_s(&m_handles[logofs].s_buffer[strlen(m_handles[logofs].s_buffer)],
                sizeof(m_handles[logofs].s_buffer) - strlen(m_handles[logofs].s_buffer),
                "\t"
                "%d\t"
                "0x%08X%08X\t"
                "%s\t"
                "%s\t"
                "%s\n",
                seq, (ULong)gm.li.highPart, gm.li.lowPart, getSeverityText(sev), pszFunc, msg);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
   write(m_handles[logofs].s_fh, m_handles[logofs].s_buffer, (UInt)strlen(m_handles[logofs].s_buffer));
#pragma GCC diagnostic pop

   m_handles[logofs].s_linecnt++;
}

Void ELogger::writeSysLog(eloggerentry_t *pLog, Int logofs, Int logid,
                           ULongLong groupid, Severity sev, ETime &t, Long seq, cpStr pszFunc, cpChar msg)
{
   EMutexLock l(m_handles[logofs].s_mutex);

   epc_sprintf_s(m_handles[logofs].s_buffer, sizeof(m_handles[logofs].s_buffer),
                "<%s> [%s] %s", getSeverityText(sev), pszFunc, msg);

   Int priority = LOG_USER | (sev == Error ? LOG_ERR : sev == Warning ? LOG_WARNING : LOG_INFO);

   openlog(pLog->s_filenamemask, LOG_NDELAY | LOG_PID | LOG_CONS, LOG_USER);
   //syslog(priority, m_handles[logofs].s_buffer);
   syslog(priority, "<%s> [%s] %s", getSeverityText(sev), pszFunc, msg);
}

ELogger::eloggerentry_t *ELogger::findLog(Int logid, Int *plogofs)
{
   Int ofs;

   for (ofs = 0; ofs < ELOGGER_MAX_LOGS && m_pCtrl->s_logs[ofs].s_logid != -1; ofs++)
   {
      if (logid == m_pCtrl->s_logs[ofs].s_logid)
      {
         if (plogofs)
            *plogofs = ofs;
         return &m_pCtrl->s_logs[ofs];
      }
   }

   return NULL;
}

Void ELogger::log(ELogger::ELoggerQueueMessage &msg)
{
   int logofs = -1;
   eloggerentry_t *pLog = m_pThis->findLog(msg.getLogId(), &logofs);
   if (pLog == NULL)
      throw ELoggerError_LogNotFound(msg.getLogId());

   m_pThis->writeFile(pLog, logofs, msg.getLogId(), msg.getGroupId(), msg.getSeverity(),
                      msg.getTime(), msg.getSequence(), msg.getFunction(), msg.getMessage());
}
