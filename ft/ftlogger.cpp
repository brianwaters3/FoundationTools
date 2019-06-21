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

#include <vector>
#include <stdarg.h>

#include "ftlogger.h"
#include "ftatomic.h"
#include "ftsynch.h"
#include "ftsynch2.h"
#include "ftinternal.h"

#if defined(FT_WINDOWS)
#include <io.h>
#elif defined(FT_GCC)
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTLoggerError_AlreadyExists::FTLoggerError_AlreadyExists(Int err)
{
    setSevere();
    setTextf("Log entry already exists for log [%d].", err);
}

FTLoggerError_LogNotFound::FTLoggerError_LogNotFound(Int err)
{
    setSevere();
    setTextf("Log [%d] not found.", err);
}

FTLoggerError_UnableToOpenLogFile::FTLoggerError_UnableToOpenLogFile(Int err, cpStr msg)
{
    setSevere();
    setTextf("Error opening [%s] - ", msg);
    appendLastOsError(err);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

cpStr FTLogger::m_pszSeverity[] = { "Undefined", "Error", "Warning", "Info", "Debug" };

class FTLoggerControl : public FTStatic
{
public:
    FTLoggerControl() {}

    ~FTLoggerControl() {}

    virtual Void init(FTGetOpt &opt)
    {
        m_logCtrl = new FTLogger();
        m_logCtrl->init(opt);
    }

    virtual Void uninit()
    {
        m_logCtrl->uninit();
        delete m_logCtrl;
    }

    virtual Int getInitType() { return STATIC_INIT_TYPE_PRIORITY; }
private:
    FTLogger *m_logCtrl;
};

FTLoggerControl _logCtrl;
FTLogger* FTLogger::m_pThis = NULL;

Void FTLogger::setLoggerPtr(FTLogger* pThis)
{
    //_logCtrl.m_pThis = pThis;
}

FTLogger::FTLogger()
{
    m_pThis = this;

    m_writetofile = False;
    for (Int ofs=0; ofs < FTLOGGER_MAX_LOGS; ofs++)
    {
        m_handles[ofs].s_fh = -1;
        m_handles[ofs].s_currseg = -1;
        m_handles[ofs].s_linecnt = 0;
    }
}

FTLogger::~FTLogger()
{
#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
    closelog(); // close syslog
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
}

Void FTLogger::init(FTGetOpt& options)
{
    options.setPrefix(SECTION_TOOLS "/" SECTION_LOGGER_OPTIONS);
    Bool bWriteToFile = options.get(MEMBER_WRITE_TO_FILE,false);
    Int nQueueID = options.get(MEMBER_QUEUE_ID, 0);
	FTString s;
	FTQueueBase::Mode mode;

	s = options.get(MEMBER_QUEUE_MODE, "WriteOnly");
    s.tolower();

	if (s == "readonly")
		mode = FTQueueBase::ReadOnly;
	else if (s == "writeonly")
		mode = FTQueueBase::WriteOnly;
	else
		mode = FTQueueBase::ReadWrite;
    
    options.setPrefix("");

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    m_writetofile = bWriteToFile;

    if (!m_writetofile)
        m_queue.init(nQueueID, mode);

    m_sharedmem.init("FTLoggerControlBlock", 1, sizeof(ftloggerctrl_t));

    m_pCtrl = (ftloggerctrl_t*)m_sharedmem.getDataPtr();

    if (!m_pCtrl->s_initialized)
    {
        for (int ofs=0; ofs < FTLOGGER_MAX_LOGS; ofs++)
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

    for (UInt idx=0; idx < cnt; idx++)
    {
        Int logid;
        ULongLong defaultmask;
        Int maxsegments;
        Int linespersegment;
        FTString filenamemask;
        FTString s;
        LogType logtype = FTLogger::ltFile;

        logid = options.get(idx, SECTION_LOGGER, MEMBER_LOG_ID, -1);
        maxsegments = options.get(idx, SECTION_LOGGER, MEMBER_SEGMENTS, -1);
        linespersegment = options.get(idx, SECTION_LOGGER, MEMBER_LINESPERSEGMENT, -1);
        filenamemask = options.get(idx, SECTION_LOGGER, MEMBER_FILENAMEMASK, "./ftlog_%A_%S.log");
        s = options.get(idx, SECTION_LOGGER, MEMBER_LOGTYPE, "File");
        logtype = (s.tolower() == "syslog") ? ltSysLog : ltFile;
        s = options.get(idx, SECTION_LOGGER, MEMBER_DEFAULTLOGMASK, "0x0000000000000000");
#if defined(FT_WINDOWS)
        defaultmask = _strtoui64(s.c_str(), NULL, 0);
#elif defined(FT_GCC)
        defaultmask = strtoull(s.c_str(), NULL, 0);
#elif defined(FT_SOLARIS)
        defaultmask = strtoull(s.c_str(), NULL, 0);
#else
#error "Unrecoginzed platform"
#endif

        addLog(logid, defaultmask, maxsegments, linespersegment, filenamemask, logtype);

        if (options.get(idx, SECTION_LOGGER, MEMBER_FTINTERNALLOG, false))
            FoundationTools::setInternalLogId(logid);
    }
}

Void FTLogger::uninit()
{
    for (Int ofs=0; ofs < FTLOGGER_MAX_LOGS; ofs++)
    {
        if (m_handles[ofs].s_fh != -1)
        {
#if defined(FT_WINDOWS)
            _close(m_handles[ofs].s_fh);
#elif defined(FT_GCC)
            close(m_handles[ofs].s_fh);
#elif defined(FT_SOLARIS)
            close(m_handles[ofs].s_fh);
#else
#error "Unrecoginzed platform"
#endif
            m_handles[ofs].s_fh = -1;
            m_handles[ofs].s_linecnt = 0;
            m_handles[ofs].s_currseg = -1;
			m_handles[ofs].s_mutex.destroy();
        }
    }

    m_queue.destroy();
}

Void FTLogger::addLog(Int logid, ULongLong defaultmask, Int maxsegments, Int linespersegment, cpChar filenamemask, LogType logtype)
{
    if (findLog(logid) != NULL)
        return;

    Int ofs;
    for (ofs=0 ; ofs < FTLOGGER_MAX_LOGS && m_pCtrl->s_logs[ofs].s_logid != -1; ofs++);

    if (ofs == FTLOGGER_MAX_LOGS)
        throw new FTLoggerError_MaximumNumberOfLogsDefined();

    ftloggerentry_t* pLog = &m_pCtrl->s_logs[ofs];

    pLog->s_logid = logid;
    pLog->s_mask.quadPart = (LongLong)defaultmask;
    pLog->s_maxsegs = maxsegments;
    pLog->s_linesperseg = linespersegment;
    ft_strcpy_s(pLog->s_filenamemask, sizeof(pLog->s_filenamemask), filenamemask);
    pLog->s_logtype = logtype;

    m_handles[ofs].s_currseg = 0;
    m_handles[ofs].s_mutex.init(NULL);
}

Bool FTLogger::isGroupMaskEnabled(Int logid, ULongLong groupMask)
{
    ftloggerentry_t* pLog = m_pThis->findLog(logid);
    if (pLog == NULL)
        throw new FTLoggerError_LogNotFound(logid);

    return groupEnabled(pLog, groupMask);
}

Void FTLogger::enableGroupMask(Int logid, ULongLong groupMask)
{
    ftloggerentry_t* pLog = m_pThis->findLog(logid);
    if (pLog == NULL)
        throw new FTLoggerError_LogNotFound(logid);

    longinteger_t gm;
    gm.quadPart = (LongLong)groupMask;

    atomic_or(pLog->s_mask.li.lowPart, gm.li.lowPart);
    atomic_or(pLog->s_mask.li.highPart, gm.li.highPart);
}

Void FTLogger::disableGroupMask(Int logid, ULongLong groupMask)
{
    ftloggerentry_t* pLog = m_pThis->findLog(logid);
    if (pLog == NULL)
        throw new FTLoggerError_LogNotFound(logid);

    longinteger_t gm;
    gm.quadPart = (LongLong)groupMask;

    atomic_and(pLog->s_mask.li.lowPart, ~gm.li.lowPart);
    atomic_and(pLog->s_mask.li.highPart, ~gm.li.highPart);
}

Void FTLogger::setGroupMask(Int logid, ULongLong groupMask)
{
    ftloggerentry_t* pLog = m_pThis->findLog(logid);
    if (pLog == NULL)
        throw new FTLoggerError_LogNotFound(logid);

    pLog->s_mask.quadPart = (LongLong)groupMask;
}

ULongLong FTLogger::getGroupMask(Int logid)
{
    ftloggerentry_t* pLog = m_pThis->findLog(logid);
    if (pLog == NULL)
        throw new FTLoggerError_LogNotFound(logid);

    return (ULongLong)pLog->s_mask.quadPart;
}

Void FTLogger::logDebug(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...)
{
    if (logid == -1)
        return;

    va_list args;
    va_start(args, pszText);
    log(logid, groupid, FTLogger::Debug, pszFunc, pszText, args);
    va_end(args);
}

Void FTLogger::logInfo(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...)
{
    if (logid == -1)
        return;

    va_list args;
    va_start(args, pszText);
    log(logid, groupid, FTLogger::Info, pszFunc, pszText, args);
    va_end(args);
}

Void FTLogger::logWarning(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...)
{
    if (logid == -1)
        return;

    va_list args;
    va_start(args, pszText);
    log(logid, groupid, FTLogger::Warning, pszFunc, pszText, args);
    va_end(args);
}

Void FTLogger::logError(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...)
{
    if (logid == -1)
        return;

    va_list args;
    va_start(args, pszText);
    log(logid, groupid, FTLogger::Error, pszFunc, pszText, args);
    va_end(args);
}

Void FTLogger::log(Int logid, ULongLong groupid, Severity esev, cpStr pszFunc, cpStr pszText, ...)
{
    if (logid == -1)
        return;

    va_list args;
    va_start(args, pszText);
    log(logid, groupid, esev, pszFunc, pszText, args);
    va_end(args);
}

Void FTLogger::log(Int logid, ULongLong groupid, Severity esev, cpStr pszFunc, cpStr pszText, va_list& args)
{
    if (logid == -1)
        return;

    Int logofs = -1;
    ftloggerentry_t* pLog = m_pThis->findLog(logid, &logofs);
    if (pLog == NULL)
        throw new FTLoggerError_LogNotFound(logid);

    if (groupEnabled(pLog,groupid) || esev == FTLogger::Error)
    {
        Char szBuff[FTLOGGER_BUFFER_SIZE];
        ft_vsnprintf_s(szBuff, sizeof(szBuff), pszText, args);
        FTTime t;

        if (m_pThis->m_writetofile)
        {
            if (pLog->s_logtype == ltSysLog)
                m_pThis->writeSysLog(pLog, logofs, logid, groupid, esev, t,
                    FTSynchObjects::getSynchObjCtrlPtr()->incSequence(), pszFunc, szBuff);
            else
                m_pThis->writeFile(pLog, logofs, logid, groupid, esev, t,
                    FTSynchObjects::getSynchObjCtrlPtr()->incSequence(), pszFunc, szBuff);
        }
        else
            m_pThis->writeQueue(pLog, logofs, logid, groupid, esev, t,
                FTSynchObjects::getSynchObjCtrlPtr()->incSequence(), pszFunc, szBuff);
    }
}

Bool FTLogger::isLogIdValid(Int logid)
{
    return m_pThis->findLog(logid) == NULL ? False : True;
}

Void FTLogger::buildFileName(ftloggerentry_t* pLog, ftloggerloghandle_t& h, FTString &s)
{
    Char mask[FT_FILENAME_MAX];
    Bool inPercent = False;
    Int val[2] = {0,0};
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
                val[valofs++] = FoundationTools::getApplicationId();
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

Void FTLogger::setNextSegment(ftloggerentry_t* pLog, ftloggerloghandle_t& h)
{
    Int lastSeg = 0;
    time_t lastTime = 0;
    FTString s;
    struct stat st;
    for (h.s_currseg=0; h.s_currseg < pLog->s_maxsegs; h.s_currseg++)
    {
        buildFileName(pLog, h, s);
        if (stat(s,&st) == 0 && st.st_mtime >= lastTime)
        {
            lastSeg = h.s_currseg;
            lastTime = st.st_mtime;
        }
    }

    if (++lastSeg >= pLog->s_maxsegs)
        lastSeg = 0;

    h.s_currseg = lastSeg;
}

Void FTLogger::verifyHandle(ftloggerentry_t* pLog, ftloggerloghandle_t& h)
{
    if (h.s_fh == -1) // need to initialize
        setNextSegment(pLog, h);

    if (h.s_linecnt >= pLog->s_linesperseg && pLog->s_linesperseg != -1)
    {
        if (h.s_fh != -1)
        {
#if defined(FT_WINDOWS)
            _close(h.s_fh);
#elif defined(FT_GCC)
            close(h.s_fh);
#elif defined(FT_SOLARIS)
            close(h.s_fh);
#else
#error "Unrecoginzed platform"
#endif
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
        FTString s;

        buildFileName(pLog, h, s);

#if defined(FT_WINDOWS)
        oflag = _O_CREAT | _O_TRUNC | _O_TEXT | _O_RDWR;
        pmode = _S_IREAD | _S_IWRITE;
        if (_sopen_s(&h.s_fh, s, oflag, _SH_DENYNO, pmode) != 0)
        {
            h.s_fh = -1;
            throw new FTLoggerError_UnableToOpenLogFile(errno, s);
        }
#elif defined(FT_GCC)
        oflag = O_CREAT | O_TRUNC | O_RDWR;
        pmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        h.s_fh = open(s, oflag, pmode);
        if (h.s_fh == -1)
            throw new FTLoggerError_UnableToOpenLogFile(errno, s);
#elif defined(FT_SOLARIS)
        oflag = O_CREAT | O_TRUNC | O_RDWR;
        pmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        h.s_fh = open(s, oflag, pmode);
        if (h.s_fh == -1)
            throw new FTLoggerError_UnableToOpenLogFile(errno, s);
#else
#error "Unrecoginzed platform"
#endif
    }
}

Void FTLogger::writeQueue(ftloggerentry_t* pLog, Int logofs, Int logid,
        ULongLong groupid, Severity esev, FTTime& t, Long seq, cpStr pszFunc, cpChar msg)
{
    FTLoggerQueueMessage qmsg;

    qmsg.setMsgType(1);
    qmsg.set(logid, groupid, esev, t, seq, pszFunc, msg);

    m_queue.push(qmsg);
}

Void FTLogger::writeFile(ftloggerentry_t* pLog, Int logofs, Int logid,
        ULongLong groupid, Severity sev, FTTime& t, Long seq, cpStr pszFunc, cpChar msg)
{
    longinteger_t gm;
    //Char buffer[4096];

    FTMutexLock l(m_handles[logofs].s_mutex);

    verifyHandle(pLog, m_handles[logofs]);

    t.Format(m_handles[logofs].s_buffer, sizeof(m_handles[logofs].s_buffer), "%F %H:%M:%S.%0", True);
    gm.quadPart = (LongLong)groupid;

    ft_sprintf_s(&m_handles[logofs].s_buffer[strlen(m_handles[logofs].s_buffer)],
        sizeof(m_handles[logofs].s_buffer) - strlen(m_handles[logofs].s_buffer),
        "\t"
        "%d\t"
        "0x%08X%08X\t"
        "%s\t"
        "%s\t"
        "%s\n",
        seq, (ULong)gm.li.highPart, gm.li.lowPart, getSeverityText(sev), pszFunc, msg);

#if defined(FT_WINDOWS)
    _write(m_handles[logofs].s_fh, m_handles[logofs].s_buffer, (UInt)strlen(m_handles[logofs].s_buffer));
#elif defined(FT_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    write(m_handles[logofs].s_fh, m_handles[logofs].s_buffer, (UInt)strlen(m_handles[logofs].s_buffer));
#pragma GCC diagnostic pop
#elif defined(FT_SOLARIS)
    write(m_handles[logofs].s_fh, m_handles[logofs].s_buffer, (UInt)strlen(m_handles[logofs].s_buffer));
#else
#error "Unrecoginzed platform"
#endif

    m_handles[logofs].s_linecnt++;
}

Void FTLogger::writeSysLog(ftloggerentry_t* pLog, Int logofs, Int logid,
        ULongLong groupid, Severity sev, FTTime& t, Long seq, cpStr pszFunc, cpChar msg)
{
    FTMutexLock l(m_handles[logofs].s_mutex);

    ft_sprintf_s(m_handles[logofs].s_buffer, sizeof(m_handles[logofs].s_buffer),
        "<%s> [%s] %s", getSeverityText(sev), pszFunc, msg);

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
    Int priority = LOG_USER | (
        sev == Error ? LOG_ERR :
        sev == Warning ? LOG_WARNING :
        LOG_INFO
    );

    openlog(pLog->s_filenamemask, LOG_NDELAY|LOG_PID|LOG_CONS, LOG_USER);
    //syslog(priority, m_handles[logofs].s_buffer);
    syslog(priority, "<%s> [%s] %s", getSeverityText(sev), pszFunc, msg);
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
}

FTLogger::ftloggerentry_t* FTLogger::findLog(Int logid, Int* plogofs)
{
    Int ofs;

    for (ofs = 0; ofs < FTLOGGER_MAX_LOGS && m_pCtrl->s_logs[ofs].s_logid != -1; ofs++)
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

Void FTLogger::log(FTLogger::FTLoggerQueueMessage& msg)
{
    int logofs = -1;
    ftloggerentry_t* pLog = m_pThis->findLog(msg.getLogId(), &logofs);
    if (pLog == NULL)
        throw new FTLoggerError_LogNotFound(msg.getLogId());

    m_pThis->writeFile(pLog, logofs, msg.getLogId(), msg.getGroupId(), msg.getSeverity(),
            msg.getTime(), msg.getSequence(), msg.getFunction(), msg.getMessage());
}
