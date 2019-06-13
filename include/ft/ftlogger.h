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

#ifndef __ftlogger_h_included
#define __ftlogger_h_included

#include "ftbase.h"
#include "ftstring.h"
#include "fttime.h"
#include "ftshmem.h"
#include "ftqpub.h"
#include "ftgetopt.h"
#include "ftstatic.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR(FTLoggerError_MaximumNumberOfLogsDefined);
DECLARE_ERROR_ADVANCED2(FTLoggerError_AlreadyExists);
DECLARE_ERROR_ADVANCED2(FTLoggerError_LogNotFound);
DECLARE_ERROR_ADVANCED3(FTLoggerError_UnableToOpenLogFile);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FoundationTools;
namespace Velocity { class BasicApplication; class LogicModuleInstance; };

#define FTLOGGER_BUFFER_SIZE    8192
#define FTLOGGER_MAX_LOGS       128

#define FTLOG_RECORDID          1

class FTLogger : public FTStatic
{
    friend class FoundationTools;
    friend class Velocity::BasicApplication;
    friend class Velocity::LogicModuleInstance;

public:
    enum Severity { Error=1, Warning=2, Info=3, Debug=4 };
    enum LogType { ltFile=1, ltSysLog=2 };

protected:
    typedef struct
    {
        Int s_logid;
        longinteger_t s_mask;
        Int s_maxsegs;
        Int s_linesperseg;
        Char s_filenamemask[FT_FILENAME_MAX];
        LogType s_logtype;
    } ftloggerentry_t;

    typedef struct
    {
        Bool s_initialized;
        ftloggerentry_t s_logs[FTLOGGER_MAX_LOGS];
    } ftloggerctrl_t;

    class ftloggerloghandle_t
    {
    public:
        ftloggerloghandle_t() : s_mutex(false) {}

        FTMutex s_mutex;
        Int s_fh;
        Int s_currseg;
        Int s_linecnt;
        Char s_buffer[FTLOGGER_BUFFER_SIZE];
    };

public:
    class FTLoggerQueueMessage : public FTQueueMessage
    {
    public:
        FTLoggerQueueMessage()
        {
            setMsgType(FTLOG_RECORDID);

            m_logid = 0;
            m_groupid = 0;
            m_severity = Info;
            m_sequence = 0;
            memset(m_func, 0, sizeof(m_func));
            memset(m_msg, 0, sizeof(m_msg));
        }

        ~FTLoggerQueueMessage()
        {
        }

        virtual Void getLength(ULong &length)
        {
            FTQueueMessage::getLength(length);
            elementLength(m_time, length);
            elementLength(m_logid, length);
            elementLength(m_groupid, length);
            elementLength((Long)m_severity, length);
            elementLength(m_sequence, length);
            elementLength(m_func, length);
            elementLength(m_msg, length);
        }
        virtual Void serialize(pVoid pBuffer, ULong& nOffset)
        {
            FTQueueMessage::serialize(pBuffer, nOffset);
            pack(m_time, pBuffer, nOffset);
            pack(m_logid, pBuffer, nOffset);
            pack(m_groupid, pBuffer, nOffset);
            pack((Long)m_severity, pBuffer, nOffset);
            pack(m_sequence, pBuffer, nOffset);
            pack(m_func, pBuffer, nOffset);
            pack(m_msg, pBuffer, nOffset);
        }
        virtual Void unserialize(pVoid pBuffer, ULong& nOffset)
        {
			Long severity;

            FTQueueMessage::unserialize(pBuffer, nOffset);
            unpack(m_time, pBuffer, nOffset);
            unpack(m_logid, pBuffer, nOffset);
            unpack(m_groupid, pBuffer, nOffset);
            unpack(severity, pBuffer, nOffset);
			m_severity = (FTLogger::Severity)severity;
            unpack(m_sequence, pBuffer, nOffset);
            unpack(m_func, pBuffer, nOffset);
            unpack(m_msg, pBuffer, nOffset);
        }

        Void set(Int logid, ULongLong groupid, FTLogger::Severity esev,
                FTTime& t, Long seq, cpStr func, cpStr msg)
        {
            m_time = t;
            m_logid = logid;
            m_groupid = groupid;
            m_severity = esev;
            m_sequence = seq;
            ft_strcpy_s(m_func, sizeof(m_func), func);
            ft_strcpy_s(m_msg, sizeof(m_msg), msg);
        }

        FTTime& getTime() { return m_time; }
        Int getLogId() { return m_logid; }
        ULongLong getGroupId() { return m_groupid; }
        FTLogger::Severity getSeverity() { return m_severity; }
        Long getSequence() { return m_sequence; }
        cpStr getFunction() { return m_func; }
        cpStr getMessage() { return m_msg; }

    private:
        FTTime m_time;
        Long m_logid;
        ULongLong m_groupid;
        FTLogger::Severity m_severity;
        Long m_sequence;
        Char m_func[FT_FILENAME_MAX];
        Char m_msg[2048];
    };

    static Bool isGroupMaskEnabled(Int logid, ULongLong groupMask);
    static Void enableGroupMask(Int logid, ULongLong groupMask);
    static Void disableGroupMask(Int logid, ULongLong groupMask);

    static Void setGroupMask(Int logid, ULongLong newGroupMask);
    static ULongLong getGroupMask(Int logid);

    static Void log(FTLoggerQueueMessage& msg);
    static Void logInfo(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...);
    static Void logError(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...);
    static Void logWarning(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...);
    static Void logDebug(Int logid, ULongLong groupid, cpStr pszFunc, cpStr pszText, ...);
    static Void log(Int logid, ULongLong groupid, Severity esev, cpStr pszFunc, cpStr pszText, ...);

    static Bool isLogIdValid(Int logid);

    static Void setLoggerPtr(FTLogger* pThis);
    FTLogger* getLoggerPtr() { return m_pThis; }

    FTLogger();
    ~FTLogger();

    virtual Int getInitType() { return STATIC_INIT_TYPE_PRIORITY; }
    Void init(FTGetOpt& options);
    Void uninit();

    cpStr getSeverityText(Severity eSeverity) { return m_pszSeverity[eSeverity]; }

    class FTLoggerQueue : public FTQueuePublic
    {
    public:
        FTLoggerQueueMessage& getMsg()
        {
            return m_msg;
        }

    protected:
        FTQueueMessage* allocMessage(Long msgType)
        {
            if (msgType == 1)
                return &m_msg;
            return NULL;
        }

    private:
        FTLoggerQueueMessage m_msg;
    };

protected:
    static cpStr m_pszSeverity[];
    static FTLogger* m_pThis;

    Void addLog(Int logid, ULongLong defaultmask, Int maxsegments, Int linespersegment, cpChar filename, LogType logtype);
    ftloggerentry_t* findLog(Int logid, Int* plogofs=NULL);

    static Void log(Int logid, ULongLong groupid, Severity esev, cpStr pszFunc, cpStr pszText, va_list& args);
    static Bool groupEnabled(ftloggerentry_t* pLog, ULongLong group)
    {
        return ((pLog->s_mask.quadPart & group) == 0) ? False : True;
    }

private:
    Void setNextSegment(ftloggerentry_t* pLog, ftloggerloghandle_t& h);
    Void buildFileName(ftloggerentry_t* pLog, ftloggerloghandle_t& h, FTString &s);
    Void verifyHandle(ftloggerentry_t* pLog, ftloggerloghandle_t& h);
    Void writeFile(ftloggerentry_t* pLog, Int logofs, Int logid, ULongLong groupid,
            Severity esev, FTTime& t, Long seq, cpStr pszFunc, cpChar msg);
    Void writeSysLog(ftloggerentry_t* pLog, Int logofs, Int logid, ULongLong groupid,
            Severity esev, FTTime& t, Long seq, cpStr pszFunc, cpChar msg);
    Void writeQueue(ftloggerentry_t* pLog, Int logofs, Int logid, ULongLong groupid,
            Severity esev, FTTime& t, Long seq, cpStr pszFunc, cpChar msg);

    Bool m_writetofile;
    ftloggerloghandle_t m_handles[FTLOGGER_MAX_LOGS];
    FTLoggerQueue m_queue;
    FTSharedMemory m_sharedmem;
    ftloggerctrl_t* m_pCtrl;
};

#endif // #define __ftlogger_h_included
