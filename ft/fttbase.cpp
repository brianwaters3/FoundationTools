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

#include "fttbase.h"
#include "ftatomic.h"

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
#include <sched.h>
#include <time.h>
#include <pthread.h>
#elif defined(FT_SOLARIS)
#include <sched.h>
#include <time.h>
#include <thread.h>
#else
#error "Unrecoginzed platform"
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FTThreadError_UnableToResume::FTThreadError_UnableToResume(): FTError()
{
    setSevere();
    setText("Error resuming thread ");
    appendLastOsError();
}

FTThreadError_UnableToSuspend::FTThreadError_UnableToSuspend(): FTError()
{
    setSevere();
    setText("Error suspending thread ");
    appendLastOsError();
}

FTThreadError_UnableToInitialize::FTThreadError_UnableToInitialize(): FTError()
{
    setSevere();
    setText("Error initializing thread ");
    appendLastOsError();
}

FTThreadTimerError_UnableToRegisterTimerHandler::FTThreadTimerError_UnableToRegisterTimerHandler()
{
    setSevere();
    setTextf("%s: Error registering timer handler - ", Name());
    appendLastOsError();
}

FTThreadTimerError_UnableToInitialize::FTThreadTimerError_UnableToInitialize()
{
    setSevere();
    setTextf("%s: Error initializing timer - ", Name());
    appendLastOsError();
}

FTThreadTimerError_NotInitialized::FTThreadTimerError_NotInitialized()
{
    setSevere();
    setTextf("%s: Error timer not initialized - ", Name());
}

FTThreadTimerError_UnableToStart::FTThreadTimerError_UnableToStart()
{
    setSevere();
    setTextf("%s: Error starting timer - ", Name());
    appendLastOsError();
}

FTThreadTimerError_UnableToStop::FTThreadTimerError_UnableToStop()
{
    setSevere();
    setTextf("%s: Error stopping timer - ", Name());
    appendLastOsError();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FTThreadPtrList FTThreadBasic::m_thrdCtl;
FTMutex FTThreadBasic::m_thrdCtlMutex(False);

Void FTThreadBasic::Initialize()
{
    m_thrdCtlMutex.init(NULL);
}

Void FTThreadBasic::UnInitialize()
{
    m_thrdCtlMutex.destroy();
}

FTThreadBasic::FTThreadBasic()
    : m_suspended(0, 1000), m_suspendSem(0, 1)
{
    m_state = FTThreadBasic::rsWaitingToRun;
    m_suspendCnt = 1;
    m_arg = NULL;
    m_exitCode = 0;
    m_keepgoing = True;
    m_updatestatemanually = False;

#if defined(FT_WINDOWS)
    m_thread = NULL;
    m_threadid = NULL;
#elif defined(FT_GCC)
    m_initialized = False;
#elif defined(FT_SOLARIS)
    m_initialized = False;
#else
#error "Unrecoginzed platform"
#endif

    FTMutexLock lc(m_thrdCtlMutex);
    m_thrdCtl.push_back(this);
}

FTThreadBasic::~FTThreadBasic()
{
    if (isRunning())
    {
        _shutdown();
        join();
    }

    FTMutexLock l(m_mutex);

    FTMutexLock lc(m_thrdCtlMutex);
    m_thrdCtl.remove(this);
}

FTThreadBasic *FTThreadBasic::findCurrentThread()
{
#if defined(FT_WINDOWS)
    Dword threadid = GetCurrentThreadId();
#elif defined(FT_GCC)
    pthread_t threadid = pthread_self();
#elif defined(FT_SOLARIS)
    pthread_t threadid = pthread_self();
#else
#error "Unrecoginzed platform"
#endif
    FTThreadPtrList::iterator iter;

    FTMutexLock l(m_thrdCtlMutex);

    for (iter = m_thrdCtl.begin(); iter != m_thrdCtl.end(); iter++)
    {
#if defined(FT_WINDOWS)
        FTMutexLock l((*iter)->m_mutex);
        if ((*iter)->isInitialized())
        {
            if ((*iter)->m_threadid == threadid)
                return (*iter);
        }
#elif defined(FT_GCC)
        if (pthread_equal((*iter)->m_thread, threadid))
            return (*iter);
#elif defined(FT_SOLARIS)
        if (pthread_equal((*iter)->m_thread, threadid))
            return (*iter);
#else
#error "Unrecoginzed platform"
#endif
    }

    return NULL;
}

Void FTThreadBasic::_shutdown()
{
    FTMutexLock l(m_mutex);

    if (isRunning())
    {
        suspend();
        shutdown();
        resume();
    }
}

Void FTThreadBasic::init(pVoid arg, Bool suspended, Dword stackSize)
{
    {
        FTMutexLock l(m_mutex);

        if (isInitialized())
            throw new FTThreadError_AlreadyInitialized();

        m_arg = arg;
#if defined(FT_WINDOWS)
        m_thread = CreateThread(NULL, stackSize, FTThreadBasic::_threadProc, (pVoid)this, CREATE_SUSPENDED, &m_threadid);

        if (m_thread == NULL)
            throw new FTThreadError_UnableToInitialize();
#elif defined(FT_GCC)
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        if (pthread_create(&m_thread, &attr, _threadProc, (pVoid)this) != 0)
            throw new FTThreadError_UnableToInitialize();

        m_initialized = True;
#elif defined(FT_SOLARIS)
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

#pragma error_messages(off, badargtype2w)
        if (pthread_create(&m_thread, &attr, _threadProc, (pVoid)this) != 0)
#pragma error_messages(on, badargtype2w)
            throw new FTThreadError_UnableToInitialize();

        m_initialized = True;
#else
#error "Unrecoginzed platform"
#endif

        if (suspended)
            m_suspendCnt++;

        resume();
    }

    while (m_suspendCnt == 0 && isWaitingToRun())
        yield();
}

Bool FTThreadBasic::isInitialized()
{
#if defined(FT_WINDOWS)
    return m_thread == NULL ? False : True;
#elif defined(FT_GCC)
    return m_initialized;
#elif defined(FT_SOLARIS)
    return m_initialized;
#else
#error "Unrecoginzed platform"
#endif
}

Void FTThreadBasic::resume()
{
    if (!isInitialized())
        throw new FTThreadError_NotInitialized();

    {
//        FTMutexLock l(m_mutex);

        if (m_suspendCnt > 0)
        {
            m_suspendCnt--;
            if (m_suspendCnt == 0)
            {
                m_suspendSem.Increment();
#if defined(FT_WINDOWS)
                if (ResumeThread(m_thread) ==  - 1)
                    throw new FTThreadError_UnableToResume();
#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
                // incrementing m_suspendSem should be sufficient
#else
#error "Unrecoginzed platform"
#endif
            }
        }
    }

    yield();
}

Void FTThreadBasic::suspend()
{
    int suspendCnt;

    {
        FTMutexLock l(m_mutex);
        m_suspendCnt++;
        suspendCnt = m_suspendCnt;
    }

    if (suspendCnt == 1)
        m_suspended.Decrement();
}

Void FTThreadBasic::_suspend()
{
    m_suspended.Increment();
    m_suspendSem.Decrement();
}

Void FTThreadBasic::join()
{
    {
        FTMutexLock l(m_mutex);
        if (!isRunning() && !isSuspended())
            return ;
    }
#if defined(FT_WINDOWS)
    WaitForSingleObject(m_thread, INFINITE);
#elif defined(FT_GCC)
    pVoid value;
    pthread_join(m_thread, &value);
#elif defined(FT_SOLARIS)
    pVoid value;
    pthread_join(m_thread, &value);
#else
#error "Unrecoginzed platform"
#endif
}

Void FTThreadBasic::sleep(int milliseconds)
{
#if defined(FT_WINDOWS)
    Sleep(milliseconds);
#elif defined(FT_GCC)
    timespec tmReq;
    tmReq.tv_sec = (time_t)(milliseconds / 1000);
    tmReq.tv_nsec = (milliseconds % 1000) * 1000 * 1000;
    // we're not interested in remaining time nor in return value
    (Void)nanosleep(&tmReq, (timespec*)NULL);
#elif defined(FT_SOLARIS)
    timespec tmReq;
    tmReq.tv_sec = (time_t)(milliseconds / 1000);
    tmReq.tv_nsec = (milliseconds % 1000) * 1000 * 1000;
    // we're not interested in remaining time nor in return value
    (Void)nanosleep(&tmReq, (timespec*)NULL);
#else
#error "Unrecoginzed platform"
#endif
}

Void FTThreadBasic::yield()
{
#if defined(FT_WINDOWS)
    Sleep(0);
#elif defined(FT_GCC)
    sched_yield();
#elif defined(FT_SOLARIS)
    sched_yield();
#else
#error "Unrecoginzed platform"
#endif
}

#if defined(FT_WINDOWS)
DWORD WINAPI FTThreadBasic::_threadProc(pVoid arg)
#elif defined(FT_GCC)
pVoid FTThreadBasic::_threadProc(pVoid arg)
#elif defined(FT_SOLARIS)
pVoid FTThreadBasic::_threadProc(pVoid arg)
#else
#error "Unrecoginzed platform"
#endif
{
    FTThreadBasic *ths = (FTThreadBasic*)arg;

    // do not continue until resumed
    ths->m_suspendSem.Decrement();

    // set to running state
    {
        FTMutexLock l(ths->m_mutex);
        if (!ths->getUpdateStateManually())
            ths->m_state = FTThreadBasic::rsRunning;
    }

    Dword ret = ths->threadProc(ths->m_arg);

    // set to not running state
    {
        FTMutexLock l(ths->m_mutex);
        if (!ths->getUpdateStateManually())
            ths->m_state = FTThreadBasic::rsDoneRunning;
    }

#if defined(FT_WINDOWS)
    return ret;
#elif defined(FT_GCC)
    return (pVoid)ret;
#elif defined(FT_SOLARIS)
    return (pVoid)ret;
#else
#error "Unrecoginzed platform"
#endif
}

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
extern "C" Void _UserSignal1Handler(int)
{
}
#elif defined(FT_SOLARIS)
extern "C" static Void _UserSignal1Handler(int)
{
}
#else
#error "Unrecoginzed platform"
#endif

Int FTThreadBasic::cancelWait()
{
    m_keepgoing = False;
#if defined(FT_WINDOWS)
    QueueUserAPC(_cancelProc, m_thread, NULL);
    return 0;
#elif defined(FT_GCC)
    //signal(SIGUSR1, _UserSignal1Handler);
    //pthread_kill(m_thread, SIGUSR1);
    return pthread_cancel(m_thread);
#elif defined(FT_SOLARIS)
    signal(SIGUSR1, _UserSignal1Handler);
    return pthread_kill(m_thread, SIGUSR1);
#else
#error "Unrecoginzed platform"
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(FTThreadBase, _FTThreadBase)
	ON_FTM_INIT()
	ON_FTM_QUIT()
	ON_FTM_SUSPEND()
    ON_FTM_TIMER()
END_MESSAGE_MAP()

FTThreadBase::FTThreadBase()
	: FTThreadBasic() // , m_queue(queueSize)
{
}

FTThreadBase::~FTThreadBase()
{
}

Bool FTThreadBase::sendMessage(UInt message, Bool wait_for_slot)
{
    Bool result = queue().push(message, wait_for_slot);
    messageQueued();
    return result;
}

Bool FTThreadBase::sendMessage(UInt message, Dword lowPart, Long highPart, Bool wait_for_slot)
{
    Bool result = queue().push(message, lowPart, highPart, wait_for_slot);
    messageQueued();
    return result;
}

Bool FTThreadBase::sendMessage(UInt message, pVoid VoidPtr, Bool wait_for_slot)
{
    Bool result = queue().push(message, VoidPtr, wait_for_slot);
    messageQueued();
    return result;
}

Bool FTThreadBase::sendMessage(UInt message, LongLong quadPart, Bool wait_for_slot)
{
    Bool result = queue().push(message, quadPart, wait_for_slot);
    messageQueued();
    return result;
}

Void FTThreadBase::init(pVoid arg, Bool suspended, Dword stackSize)
{
    FTThreadBasic::init(arg, suspended, stackSize);
    sendMessage(FTM_INIT);
}

Void FTThreadBase::quit()
{
    sendMessage(FTM_QUIT);
}

Void FTThreadBase::suspend()
{
    sendMessage(FTM_SUSPEND);
    FTThreadBasic::suspend();
}

Void FTThreadBase::onInit()
{
}

Void FTThreadBase::onQuit()
{
}

Void FTThreadBase::onSuspend()
{
    _suspend();
}

Void FTThreadBase::onTimer(FTThreadBase::Timer *ptimer)
{
    printf("FTThreadBase::onTimer (%p)\n", ptimer);

}

Void FTThreadBase::defMessageHandler(FTThreadMessage &msg)
{
}

Dword FTThreadBase::threadProc(pVoid arg)
{
    pumpMessages();
    return 0;
}

Void FTThreadBase::messageQueued()
{
}

Bool FTThreadBase::pumpMessage(FTThreadMessage &msg, Bool wait)
{
    Bool bMsg = queue().pop(msg, wait);
    if (bMsg)
        dispatch(msg);

    return bMsg;
}

Void FTThreadBase::pumpMessages()
{
    FTThreadMessage msg;

    try
    {
        while (True)
        {
            if (pumpMessage(msg))
            {
                if (msg.getMsgId() == FTM_QUIT)
                    break;
            }
            if (!keepGoing())
                break;
        }
    }
    catch (FTError *e)
    {
        printf("t1 - %s\n", e->Name());
        throw;
    }
}

Bool FTThreadBase::dispatch(FTThreadMessage &msg)
{
    Bool res = True;
    Bool keepgoing = True;
    const ftthread_msgmap_t *pMap;
    const ftthread_msgentry_t *pEntries;

    // interate through each map
    for (pMap = GetMessageMap(); pMap && pMap->pfnGetBaseMap != NULL; pMap = (*pMap->pfnGetBaseMap)())
    {
        // interate through each entry for the map
        for (pEntries = pMap->lpEntries; pEntries->nFnType != eMsgFxnEnd; pEntries++)
        {
            if (pEntries->nMessage == msg.getMsgId())
            {
                switch (pEntries->nFnType)
                {
                    case eMsgFxnA:
                        (this->*pEntries->pFn)();
                        break;
                    case eMsgFxnB:
                        (this->*(ftthread_msgfxnB_t)pEntries->pFn)(msg);
                        break;
                    case eMsgFxnC:
                        (this->*(ftthread_msgfxnC_t)pEntries->pFn)(msg.getHighPart());
                        break;
                    case eMsgFxnD:
                        (this->*(ftthread_msgfxnD_t)pEntries->pFn)(msg.getVoidPtr());
                        break;
                    default:
                        res = False;
                        break;
                }

                keepgoing = False;
                break;
            }
        }

        if (!keepgoing)
            break;
    }

    if (pMap == NULL)
        defMessageHandler(msg);

    return keepgoing ? res : False;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Long FTThreadBase::Timer::m_nextid = 0;

FTThreadBase::Timer::Timer()
{
    // assign the id
    m_id = atomic_inc(m_nextid);
    m_pThread = NULL;
    m_interval = 0;
    m_oneshot = True;

#if defined(FT_WINDOWS)
    m_handle = INVALID_HANDLE_VALUE;
#elif defined(FT_GCC)
    m_timer = NULL;
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
}

FTThreadBase::Timer::Timer(Long milliseconds, Bool oneshot)
{
    // assign the id
    m_id = atomic_inc(m_nextid);
    m_pThread = NULL;
    m_interval = milliseconds;
    m_oneshot = oneshot;

#if defined(FT_WINDOWS)
    m_handle = INVALID_HANDLE_VALUE;
#elif defined(FT_GCC)
    m_timer = NULL;
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
}

FTThreadBase::Timer::~Timer()
{
    destroy();
}

Void FTThreadBase::Timer::init(FTThreadBase* pThread)
{
    m_pThread = pThread;

#if defined(FT_WINDOWS)
    m_handle = CreateWaitableTimer(NULL, TRUE, NULL);
    if (m_handle == INVALID_HANDLE_VALUE)
        throw new FTThreadTimerError_UnableToInitialize();
#elif defined(FT_GCC)
    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = this;
    if (timer_create(CLOCK_REALTIME, &sev, &m_timer) == -1)
        throw new FTThreadTimerError_UnableToInitialize();
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
}

Void FTThreadBase::Timer::destroy()
{
#if defined(FT_WINDOWS)
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        stop();
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
#elif defined(FT_GCC)
    if (m_timer != NULL)
    {
        stop();
        timer_delete(m_timer);
        m_timer = NULL;
    }
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
}

Void FTThreadBase::Timer::start()
{
#if defined(FT_WINDOWS)
    if (m_handle == INVALID_HANDLE_VALUE)
        throw new FTThreadTimerError_NotInitialized();

    __int64 qwDueTime = -m_interval * 10000;
    LARGE_INTEGER liDueTime;

    liDueTime.LowPart  = (DWORD) (qwDueTime & 0xFFFFFFFF);
    liDueTime.HighPart = (LONG)  (qwDueTime >> 32);

    if (!SetWaitableTimer(m_handle, &liDueTime, m_oneshot ? 0 : m_interval, _timerAPC, this, FALSE))
        throw new FTThreadTimerError_UnableToStart();
#elif defined(FT_GCC)
    if (m_timer == NULL)
        throw new FTThreadTimerError_NotInitialized();

    struct itimerspec its;
    its.it_value.tv_sec = m_interval / 1000; // seconds
    its.it_value.tv_nsec = (m_interval % 1000) * 1000000; // nano-seconds
    its.it_interval.tv_sec = m_oneshot ? 0 : its.it_value.tv_sec;
    its.it_interval.tv_nsec = m_oneshot ? 0 : its.it_value.tv_nsec;
    if (timer_settime(m_timer, 0, &its, NULL) == -1)
        throw new FTThreadTimerError_UnableToStart();
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
}

Void FTThreadBase::Timer::stop()
{
#if defined(FT_WINDOWS)
    if (m_handle != INVALID_HANDLE_VALUE)
        CancelWaitableTimer(m_handle);
#elif defined(FT_GCC)
    if (m_timer != NULL)
    {
        struct itimerspec its;
        its.it_value.tv_sec = 0; // seconds
        its.it_value.tv_nsec = 0; // nano-seconds
        its.it_interval.tv_sec = its.it_value.tv_sec;
        its.it_interval.tv_nsec = its.it_value.tv_nsec;
        timer_settime(m_timer, 0, &its, NULL);
    }
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
}

#if defined(FT_WINDOWS)
VOID CALLBACK FTThreadBase::Timer::_timerAPC(pVoid arg1, DWORD arg2, DWORD arg3)
{
    FTThreadBase::Timer* pTimer = (FTThreadBase::Timer*)arg1;
    pTimer->m_pThread->sendMessage(FTM_TIMER, pTimer);
}
#elif defined(FT_GCC)
void FTThreadBase::Timer::_timerHandler(int signo, siginfo_t *pinfo, void *pcontext)
{
    FTThreadBase::Timer* pTimer = (FTThreadBase::Timer*)pinfo->si_value.sival_ptr;
    if (pTimer)
        pTimer->m_pThread->sendMessage(FTM_TIMER, pTimer);
}

FTThreadBase::TimerHandler _initTimerHandler;

Void FTThreadBase::TimerHandler::init(FTGetOpt& options)
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = FTThreadBase::Timer::_timerHandler;
    sigemptyset(&sa.sa_mask);
    int signo = SIGRTMIN;
    if (sigaction(signo, &sa, NULL) == -1)
        throw new FTThreadTimerError_UnableToRegisterTimerHandler();
}

Void FTThreadBase::TimerHandler::uninit()
{
}
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
