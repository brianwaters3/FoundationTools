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

#ifndef __fttbase_h_included
#define __fttbase_h_included

#include "ftbase.h"
#include "fterror.h"
#include "ftgetopt.h"
#include "ftstatic.h"
#include "ftsynch.h"
#include "fttq.h"

class FTThreadBasic;
class FTThreadBase;

class FTThreadError_InvalidHandle: public FTError {};
class FTThreadError_NotInitialized: public FTError {};
class FTThreadError_AlreadyInitialized: public FTError {};
class FTThreadError_UnableToResume: public FTError { public: FTThreadError_UnableToResume(); };
class FTThreadError_UnableToSuspend: public FTError { public: FTThreadError_UnableToSuspend(); };
class FTThreadError_UnableToInitialize: public FTError { public: FTThreadError_UnableToInitialize(); };

DECLARE_ERROR_ADVANCED(FTThreadTimerError_UnableToRegisterTimerHandler);
DECLARE_ERROR_ADVANCED(FTThreadTimerError_UnableToInitialize);
DECLARE_ERROR_ADVANCED(FTThreadTimerError_NotInitialized);
DECLARE_ERROR_ADVANCED(FTThreadTimerError_UnableToStart);
DECLARE_ERROR_ADVANCED(FTThreadTimerError_UnableToStop);

typedef list < FTThreadBasic * > FTThreadPtrList;

class FTThreadBasic
{
    friend class FTThreadBase;

public:
    enum RunState
    {
        rsWaitingToRun,
        rsRunning,
        rsDoneRunning
    };

    FTThreadBasic();
    virtual ~FTThreadBasic();

    virtual Dword threadProc(pVoid arg) = 0;

    static Void Initialize();
    static Void UnInitialize();

    Void init(pVoid arg, Bool suspended = False, Dword stackSize = 0);
    Void resume();
    Void join();

    static Void sleep(Int milliseconds);
    static Void yield();

    Bool isInitialized();

    Bool isSuspended()
    {
        return m_suspendCnt == 0 ? False : True;
    }

    Void setRunState(RunState state) { m_state = state; }
    RunState getRunState() { return m_state; }

    Bool isWaitingToRun()   { return m_state == FTThreadBasic::rsWaitingToRun; }
    Bool isRunning()        { return m_state == FTThreadBasic::rsRunning; }
    Bool isDoneRunning()    { return m_state == FTThreadBasic::rsDoneRunning; }

    Void setUpdateStateManually(Bool val)   { m_updatestatemanually = val; }
    Bool getUpdateStateManually()           { return m_updatestatemanually; }

    Bool keepGoing()        { return m_keepgoing; }

    virtual Void shutdown()
    {
        // This method is called in the event of the object being destroyed
        // while the thread is running.  This is intended for use for threads
        // that do not support a message queue (FTTM_QUIT will be issued) to
        // inform the thread that it needs to exit.
    }

    Int cancelWait();

protected:
    virtual Void suspend();
    Void _suspend();

private:
#if defined(FT_WINDOWS)
    HANDLE m_thread;
    Dword m_threadid;
    static DWORD WINAPI _threadProc(pVoid arg);

    static VOID CALLBACK _cancelProc(ULONG_PTR dwParam)
    {
    }
#elif defined(FT_GCC)
    Bool m_initialized;
    pthread_t m_thread;
    static pVoid _threadProc(pVoid arg);
#elif defined(FT_SOLARIS)
    Bool m_initialized;
    pthread_t m_thread;
    static pVoid _threadProc(pVoid arg);
#else
#error "Unrecoginzed platform"
#endif
    static FTThreadBasic *findCurrentThread();
    static FTThreadPtrList m_thrdCtl;
    static FTMutex m_thrdCtlMutex;

    Void _shutdown();

    FTMutex m_mutex;
    RunState m_state;
    Bool m_keepgoing;
    Bool m_updatestatemanually;
    FTSemaphore m_suspended;
    FTSemaphore m_suspendSem;
    Int m_suspendCnt;
    pVoid m_arg;
    Dword m_exitCode;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct ftthread_msgmap_t;

#define DECLARE_MESSAGE_MAP() \
protected: \
static const ftthread_msgmap_t* GetThisMessageMap(); \
virtual const ftthread_msgmap_t* GetMessageMap() const; \

class _FTThreadBase
{
    friend class FTThreadBase;

protected:
    virtual const ftthread_msgmap_t *GetMessageMap()const
    {
        return GetThisMessageMap();
    }

    static const ftthread_msgmap_t *GetThisMessageMap()
    {
        return NULL;
    }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTThreadBase : public _FTThreadBase, public FTThreadBasic
{
public:
    class Timer
#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
        : public FTStatic
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
    {
        friend class FTThreadBase;

    protected:
        Void init(FTThreadBase* pThread);

    public:
        Timer();
        Timer(Long milliseconds, Bool oneshot=False);
        ~Timer();

        Void destroy();
        Void start();
        Void stop();

		Long getInterval()				{ return m_interval; }
        Void setInterval(Long interval) { m_interval = interval; }
        Void setOneShot(Bool oneshot) { m_oneshot = oneshot; }
        Long getId() { return m_id; }

    private:
        static Long m_nextid;

        Long m_id;
        FTThreadBase* m_pThread;
        Bool m_oneshot;
        Long m_interval;
#if defined(FT_WINDOWS)
        HANDLE m_handle;
 		static VOID CALLBACK _timerAPC(PVOID arg1, DWORD arg2, DWORD arg3);
#elif defined(FT_GCC)
        timer_t m_timer;
        static void _timerHandler(int signo, siginfo_t *pinfo, void *pcontext);
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
    };

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
    class TimerHandler : FTStatic
    {
    public:
        TimerHandler() {}
        ~TimerHandler() {}

        virtual Int getInitType()       { return STATIC_INIT_TYPE_THREADS; }
        Void init(FTGetOpt& options);
        Void uninit();
    };
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif

public:
    FTThreadBase();
    ~FTThreadBase();

    Bool sendMessage(UInt message, Bool wait = True);
    Bool sendMessage(UInt message, Dword lowPart, Long highPart, Bool wait = True);
    Bool sendMessage(UInt message, pVoid voidPtr, Bool wait = True);
    Bool sendMessage(UInt message, LongLong quadPart, Bool wait = True);

    Void init(pVoid arg, Bool suspended = False, Dword stackSize = 0);
    Void quit();
    Void suspend();
    Bool pumpMessage(FTThreadMessage &msg, Bool wait=true);

    FTSemaphore& getMsgSemaphore()
    {
        return queue().semMsgs();
    }

    virtual Void onInit();
    virtual Void onQuit();
    virtual Void onSuspend();
    virtual Void onTimer(FTThreadBase::Timer *ptimer);

    virtual Void pumpMessages();
    virtual Void defMessageHandler(FTThreadMessage &msg);

    Void initTimer(Timer& t)
    {
        t.init(this);
    }

    DECLARE_MESSAGE_MAP()

protected:
	virtual FTThreadQueueBase& queue() = 0;
	virtual Void messageQueued();

private:
    Dword threadProc(pVoid arg);
    Bool dispatch(FTThreadMessage &msg);
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef Void(FTThreadBase:: *ftthread_msgfxnA_t)();
typedef Void(FTThreadBase:: *ftthread_msgfxnB_t)(FTThreadMessage &);
typedef Void(FTThreadBase:: *ftthread_msgfxnC_t)(Long);
typedef Void(FTThreadBase:: *ftthread_msgfxnD_t)(pVoid);

enum ftthread_msgfnx_e
{
    eMsgFxnEnd, eMsgFxnA, eMsgFxnB, eMsgFxnC, eMsgFxnD
};

typedef struct
{
    UInt nMessage; // message
    ftthread_msgfnx_e nFnType; // function type
    ftthread_msgfxnA_t pFn; // routine to call (or special value)
} ftthread_msgentry_t;

struct ftthread_msgmap_t
{
    const ftthread_msgmap_t *(*pfnGetBaseMap)();
    const ftthread_msgentry_t *lpEntries;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define FTM_INIT	                1
#define FTM_QUIT	                2
#define FTM_SUSPEND	                3
#define FTM_TIMER                   4
#define FTM_SOCKETSELECT_READ       5
#define FTM_SOCKETSELECT_WRITE      6
#define FTM_SOCKETSELECT_ERROR      7
#define FTM_SOCKETSELECT_EXCEPTION  8
#define FTM_USER	                10000

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
const ftthread_msgmap_t* theClass::GetMessageMap() const \
{ return GetThisMessageMap(); } \
const ftthread_msgmap_t* theClass::GetThisMessageMap() \
{ \
typedef theClass ThisClass; \
typedef baseClass TheBaseClass; \
static const ftthread_msgentry_t _msgEntries[] = \
{

#define END_MESSAGE_MAP() \
{ 0, eMsgFxnEnd, (ftthread_msgfxnA_t)0 } \
}; \
static const ftthread_msgmap_t msgMap = \
{ &TheBaseClass::GetThisMessageMap, &_msgEntries[0] }; \
return &msgMap; \
} \

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define ON_FTM_INIT() \
{ FTM_INIT, eMsgFxnA, (ftthread_msgfxnA_t)(&FTThreadBase::onInit) },

#define ON_FTM_QUIT() \
{ FTM_QUIT, eMsgFxnA, (ftthread_msgfxnA_t)(&FTThreadBase::onQuit) },

#define ON_FTM_SUSPEND() \
{ FTM_SUSPEND, eMsgFxnA, (ftthread_msgfxnA_t)(&FTThreadBase::onSuspend) },

#define ON_FTM_TIMER() \
{ FTM_TIMER, eMsgFxnD, (ftthread_msgfxnA_t)(&FTThreadBase::onTimer) },

#define ON_MESSAGE(id, memberFxn) \
{ id, eMsgFxnB, (ftthread_msgfxnA_t)(Void (FTThreadBase::*)(FTThreadMessage&))(&memberFxn) },

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif // #define __fttbase_h_included
