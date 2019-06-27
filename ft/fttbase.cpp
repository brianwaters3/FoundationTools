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

#include <sched.h>
#include <time.h>
#include <pthread.h>

#include "fttbase.h"
#include "ftatomic.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FTThreadError_UnableToResume::FTThreadError_UnableToResume() : FTError()
{
   setSevere();
   setText("Error resuming thread ");
   appendLastOsError();
}

FTThreadError_UnableToSuspend::FTThreadError_UnableToSuspend() : FTError()
{
   setSevere();
   setText("Error suspending thread ");
   appendLastOsError();
}

FTThreadError_UnableToInitialize::FTThreadError_UnableToInitialize() : FTError()
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
FTMutexPrivate FTThreadBasic::m_thrdCtlMutex(False);

Void FTThreadBasic::Initialize()
{
   m_thrdCtlMutex.init();
}

Void FTThreadBasic::UnInitialize()
{
   m_thrdCtlMutex.destroy();
}

FTThreadBasic::FTThreadBasic()
{
   m_state = FTThreadBasic::rsWaitingToRun;
   m_arg = NULL;
   m_exitCode = 0;
   m_initialized = False;

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
   pthread_t threadid = pthread_self();
   FTThreadPtrList::iterator iter;

   FTMutexLock l(m_thrdCtlMutex);

   for (iter = m_thrdCtl.begin(); iter != m_thrdCtl.end(); iter++)
   {
      if (pthread_equal((*iter)->m_thread, threadid))
         return (*iter);
   }

   return NULL;
}

Void FTThreadBasic::_shutdown()
{
   FTMutexLock l(m_mutex);

   if (isRunning())
      cancelWait();
}

Void FTThreadBasic::init(pVoid arg, Dword stackSize)
{
   {
      FTMutexLock l(m_mutex);

      if (isInitialized())
         throw FTThreadError_AlreadyInitialized();

      m_arg = arg;
      pthread_attr_t attr;

      pthread_attr_init(&attr);
      pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

      if (pthread_create(&m_thread, &attr, _threadProc, (pVoid)this) != 0)
         throw FTThreadError_UnableToInitialize();

      m_initialized = True;
   }

   while (isWaitingToRun())
      yield();
}

Bool FTThreadBasic::isInitialized()
{
   return m_initialized;
}

Void FTThreadBasic::join()
{
   {
      FTMutexLock l(m_mutex);
      if (!isRunning())
         return;
   }
   pVoid value;
   pthread_join(m_thread, &value);
}

Void FTThreadBasic::sleep(int milliseconds)
{
   timespec tmReq;
   tmReq.tv_sec = (time_t)(milliseconds / 1000);
   tmReq.tv_nsec = (milliseconds % 1000) * 1000 * 1000;
   // we're not interested in remaining time nor in return value
   (Void) nanosleep(&tmReq, (timespec *)NULL);
}

Void FTThreadBasic::yield()
{
   sched_yield();
}

pVoid FTThreadBasic::_threadProc(pVoid arg)
{
   FTThreadBasic *ths = (FTThreadBasic *)arg;

   // set to running state
   {
      FTMutexLock l(ths->m_mutex);
      ths->m_state = FTThreadBasic::rsRunning;
   }

   Dword ret = ths->threadProc(ths->m_arg);

   // set to not running state
   {
      FTMutexLock l(ths->m_mutex);
      ths->m_state = FTThreadBasic::rsDoneRunning;
   }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
   return (pVoid)ret;
#pragma GCC diagnostic pop
}

extern "C" Void _UserSignal1Handler(int)
{
}

Int FTThreadBasic::cancelWait()
{
   return pthread_cancel(m_thread);
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
    : FTThreadBasic(),
      m_arg(NULL),
      m_stacksize(0),
      m_suspendCnt(0),
      m_suspendSem(0)
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
   m_arg = arg;
   m_stacksize = stackSize;

   if (!suspended)
      start();
}

Void FTThreadBase::start()
{
   if (!isInitialized())
   {
      FTThreadBasic::init(m_arg, m_stacksize);
      sendMessage(FTM_INIT);
   }
}

Void FTThreadBase::quit()
{
   sendMessage(FTM_QUIT);
}

Void FTThreadBase::suspend()
{
   if (atomic_inc(m_suspendCnt) == 1)
      sendMessage(FTM_SUSPEND);
}

Void FTThreadBase::resume()
{
   if (atomic_dec(m_suspendCnt) == 0)
      m_suspendSem.Increment();
}

Void FTThreadBase::onInit()
{
}

Void FTThreadBase::onQuit()
{
}

Void FTThreadBase::onSuspend()
{
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
            if (msg.getMsgId() == FTM_SUSPEND)
               m_suspendSem.Decrement();
         }
      }
   }
   catch (FTError &e)
   {
      printf("t1 - %s\n", e.Name());
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
   m_timer = NULL;
}

FTThreadBase::Timer::Timer(Long milliseconds, Bool oneshot)
{
   // assign the id
   m_id = atomic_inc(m_nextid);
   m_pThread = NULL;
   m_interval = milliseconds;
   m_oneshot = oneshot;
   m_timer = NULL;
}

FTThreadBase::Timer::~Timer()
{
   destroy();
}

Void FTThreadBase::Timer::init(FTThreadBase *pThread)
{
   m_pThread = pThread;

   struct sigevent sev;
   sev.sigev_notify = SIGEV_SIGNAL;
   sev.sigev_signo = SIGRTMIN;
   sev.sigev_value.sival_ptr = this;
   if (timer_create(CLOCK_REALTIME, &sev, &m_timer) == -1)
      throw FTThreadTimerError_UnableToInitialize();
}

Void FTThreadBase::Timer::destroy()
{
   if (m_timer != NULL)
   {
      stop();
      timer_delete(m_timer);
      m_timer = NULL;
   }
}

Void FTThreadBase::Timer::start()
{
   if (m_timer == NULL)
      throw FTThreadTimerError_NotInitialized();

   struct itimerspec its;
   its.it_value.tv_sec = m_interval / 1000;              // seconds
   its.it_value.tv_nsec = (m_interval % 1000) * 1000000; // nano-seconds
   its.it_interval.tv_sec = m_oneshot ? 0 : its.it_value.tv_sec;
   its.it_interval.tv_nsec = m_oneshot ? 0 : its.it_value.tv_nsec;
   if (timer_settime(m_timer, 0, &its, NULL) == -1)
      throw FTThreadTimerError_UnableToStart();
}

Void FTThreadBase::Timer::stop()
{
   if (m_timer != NULL)
   {
      struct itimerspec its;
      its.it_value.tv_sec = 0;  // seconds
      its.it_value.tv_nsec = 0; // nano-seconds
      its.it_interval.tv_sec = its.it_value.tv_sec;
      its.it_interval.tv_nsec = its.it_value.tv_nsec;
      timer_settime(m_timer, 0, &its, NULL);
   }
}

void FTThreadBase::Timer::_timerHandler(int signo, siginfo_t *pinfo, void *pcontext)
{
   FTThreadBase::Timer *pTimer = (FTThreadBase::Timer *)pinfo->si_value.sival_ptr;
   if (pTimer)
      pTimer->m_pThread->sendMessage(FTM_TIMER, pTimer);
}

FTThreadBase::TimerHandler _initTimerHandler;

Void FTThreadBase::TimerHandler::init(FTGetOpt &options)
{
   struct sigaction sa;
   sa.sa_flags = SA_SIGINFO;
   sa.sa_sigaction = FTThreadBase::Timer::_timerHandler;
   sigemptyset(&sa.sa_mask);
   int signo = SIGRTMIN;
   if (sigaction(signo, &sa, NULL) == -1)
      throw FTThreadTimerError_UnableToRegisterTimerHandler();
}

Void FTThreadBase::TimerHandler::uninit()
{
}
