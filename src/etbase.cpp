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

#include <sched.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "etbase.h"
#include "eatomic.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EThreadError_UnableToResume::EThreadError_UnableToResume() : EError()
{
   setSevere();
   setText("Error resuming thread ");
   appendLastOsError();
}

EThreadError_UnableToSuspend::EThreadError_UnableToSuspend() : EError()
{
   setSevere();
   setText("Error suspending thread ");
   appendLastOsError();
}

EThreadError_UnableToInitialize::EThreadError_UnableToInitialize() : EError()
{
   setSevere();
   setText("Error initializing thread ");
   appendLastOsError();
}

EThreadTimerError_UnableToRegisterTimerHandler::EThreadTimerError_UnableToRegisterTimerHandler()
{
   setSevere();
   setTextf("%s: Error registering timer handler - ", Name());
   appendLastOsError();
}

EThreadTimerError_UnableToInitialize::EThreadTimerError_UnableToInitialize()
{
   setSevere();
   setTextf("%s: Error initializing timer - ", Name());
   appendLastOsError();
}

EThreadTimerError_NotInitialized::EThreadTimerError_NotInitialized()
{
   setSevere();
   setTextf("%s: Error timer not initialized - ", Name());
}

EThreadTimerError_UnableToStart::EThreadTimerError_UnableToStart()
{
   setSevere();
   setTextf("%s: Error starting timer - ", Name());
   appendLastOsError();
}

EThreadTimerError_UnableToStop::EThreadTimerError_UnableToStop()
{
   setSevere();
   setTextf("%s: Error stopping timer - ", Name());
   appendLastOsError();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EThreadPtrList EThreadBasic::m_thrdCtl;
EMutexPrivate EThreadBasic::m_thrdCtlMutex(False);

Void EThreadBasic::Initialize()
{
   m_thrdCtlMutex.init();
}

Void EThreadBasic::UnInitialize()
{
   m_thrdCtlMutex.destroy();
}

EThreadBasic::EThreadBasic()
{
   m_state = EThreadBasic::rsWaitingToRun;
   m_arg = NULL;
   m_exitCode = 0;
   m_initialized = False;

   EMutexLock lc(m_thrdCtlMutex);
   m_thrdCtl.push_back(this);
}

EThreadBasic::~EThreadBasic()
{
   if (isRunning())
   {
      _shutdown();
      join();
   }

   EMutexLock l(m_mutex);

   EMutexLock lc(m_thrdCtlMutex);
   m_thrdCtl.remove(this);
}

EThreadBasic *EThreadBasic::findCurrentThread()
{
   pthread_t threadid = pthread_self();
   EThreadPtrList::iterator iter;

   EMutexLock l(m_thrdCtlMutex);

   for (iter = m_thrdCtl.begin(); iter != m_thrdCtl.end(); iter++)
   {
      if (pthread_equal((*iter)->m_thread, threadid))
         return (*iter);
   }

   return NULL;
}

Void EThreadBasic::_shutdown()
{
   EMutexLock l(m_mutex);

   if (isRunning())
      cancelWait();
}

Void EThreadBasic::init(pVoid arg, size_t stackSize)
{
   {
      EMutexLock l(m_mutex);

      if (isInitialized())
         throw EThreadError_AlreadyInitialized();

      m_arg = arg;
      pthread_attr_t attr;

      pthread_attr_init(&attr);
      pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
      if (stackSize != 0)
         pthread_attr_setstacksize(&attr, stackSize);

      if (pthread_create(&m_thread, &attr, _threadProc, (pVoid)this) != 0)
         throw EThreadError_UnableToInitialize();

      m_initialized = True;
   }

   while (isWaitingToRun())
      yield();
}

Bool EThreadBasic::isInitialized()
{
   return m_initialized;
}

Void EThreadBasic::join()
{
   {
      EMutexLock l(m_mutex);
      if (!isRunning())
         return;
   }
   pVoid value;
   pthread_join(m_thread, &value);
}

Void EThreadBasic::sleep(int milliseconds)
{
   timespec tmReq;
   tmReq.tv_sec = (time_t)(milliseconds / 1000);
   tmReq.tv_nsec = (milliseconds % 1000) * 1000 * 1000;
   // we're not interested in remaining time nor in return value
   (Void) nanosleep(&tmReq, (timespec *)NULL);
}

Void EThreadBasic::yield()
{
   sched_yield();
}

pVoid EThreadBasic::_threadProc(pVoid arg)
{
   EThreadBasic *ths = (EThreadBasic *)arg;

   // set to running state
   {
      EMutexLock l(ths->m_mutex);
      ths->m_state = EThreadBasic::rsRunning;
   }

   Dword ret = ths->threadProc(ths->m_arg);

   // set to not running state
   {
      EMutexLock l(ths->m_mutex);
      ths->m_state = EThreadBasic::rsDoneRunning;
   }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
   return (pVoid)ret;
#pragma GCC diagnostic pop
}

extern "C" Void _UserSignal1Handler(int)
{
}

Int EThreadBasic::cancelWait()
{
   return pthread_cancel(m_thread);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(EThreadBase, _EThreadBase)
   ON_EM_INIT()
   ON_EM_QUIT()
   ON_EM_SUSPEND()
   ON_EM_TIMER()
END_MESSAGE_MAP()

EThreadBase::EThreadBase()
    : EThreadBasic(),
      m_tid(-1),
      m_arg(NULL),
      m_stacksize(0),
      m_suspendCnt(0),
      m_suspendSem(0)
{
}

EThreadBase::~EThreadBase()
{
}

Bool EThreadBase::sendMessage(UInt message, Bool wait_for_slot)
{
   Bool result = queue().push(message, wait_for_slot);
   messageQueued();
   return result;
}

Bool EThreadBase::sendMessage(UInt message, Dword lowPart, Long highPart, Bool wait_for_slot)
{
   Bool result = queue().push(message, lowPart, highPart, wait_for_slot);
   messageQueued();
   return result;
}

Bool EThreadBase::sendMessage(UInt message, pVoid VoidPtr, Bool wait_for_slot)
{
   Bool result = queue().push(message, VoidPtr, wait_for_slot);
   messageQueued();
   return result;
}

Bool EThreadBase::sendMessage(UInt message, LongLong quadPart, Bool wait_for_slot)
{
   Bool result = queue().push(message, quadPart, wait_for_slot);
   messageQueued();
   return result;
}

Bool EThreadBase::sendMessage(EThreadMessage &message, Bool wait_for_slot)
{
   Bool result = queue().push(message.getMsgId(), message.getQuadPart(), wait_for_slot);
   messageQueued();
   return result;
}

Void EThreadBase::init(pVoid arg, Bool suspended, size_t stackSize)
{
   m_arg = arg;
   m_stacksize = stackSize;

   if (!suspended)
      start();
}

Void EThreadBase::start()
{
   if (!isInitialized())
   {
      EThreadBasic::init(m_arg, m_stacksize);
      sendMessage(EM_INIT);
   }
}

Void EThreadBase::quit()
{
   sendMessage(EM_QUIT);
}

Void EThreadBase::suspend()
{
   if (atomic_inc(m_suspendCnt) == 1)
      sendMessage(EM_SUSPEND);
}

Void EThreadBase::resume()
{
   if (atomic_dec(m_suspendCnt) == 0)
      m_suspendSem.Increment();
}

pid_t EThreadBase::getThreadId()
{
   if (m_tid == -1)
      m_tid = syscall(SYS_gettid);
   return m_tid;
}

Void EThreadBase::onInit()
{
}

Void EThreadBase::onQuit()
{
}

Void EThreadBase::onSuspend()
{
}

Void EThreadBase::onTimer(EThreadBase::Timer *ptimer)
{
   std::cout << "EThreadBase::onTimer (" << static_cast<void*>(ptimer) << ")" << std::endl;
}

Void EThreadBase::defMessageHandler(EThreadMessage &msg)
{
}

Dword EThreadBase::threadProc(pVoid arg)
{
   pumpMessages();
   return 0;
}

Void EThreadBase::messageQueued()
{
}

Bool EThreadBase::pumpMessage(EThreadMessage &msg, Bool wait)
{
   Bool bMsg = queue().pop(msg, wait);
   if (bMsg)
      dispatch(msg);

   return bMsg;
}

Void EThreadBase::pumpMessages()
{
   EThreadMessage msg;

   try
   {
      while (True)
      {
         if (pumpMessage(msg))
         {
            if (msg.getMsgId() == EM_QUIT)
               break;
            if (msg.getMsgId() == EM_SUSPEND)
               m_suspendSem.Decrement();
         }
      }
   }
   catch (EError &e)
   {
      //std::cout << "t1 - " << e.Name() << std::endl;
      throw;
   }
   catch (...)
   {
      throw;
   }
}

Bool EThreadBase::dispatch(EThreadMessage &msg)
{
   Bool res = True;
   Bool keepgoing = True;
   const ethread_msgmap_t *pMap;
   const ethread_msgentry_t *pEntries;

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
               (this->*(ethread_msgfxnB_t)pEntries->pFn)(msg);
               break;
            case eMsgFxnC:
               (this->*(ethread_msgfxnC_t)pEntries->pFn)(msg.getHighPart());
               break;
            case eMsgFxnD:
               (this->*(ethread_msgfxnD_t)pEntries->pFn)(msg.getVoidPtr());
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

Long EThreadBase::Timer::m_nextid = 0;

EThreadBase::Timer::Timer()
{
   // assign the id
   m_id = atomic_inc(m_nextid);
   m_pThread = NULL;
   m_interval = 0;
   m_oneshot = True;
   m_timer = NULL;
}

EThreadBase::Timer::Timer(Long milliseconds, Bool oneshot)
{
   // assign the id
   m_id = atomic_inc(m_nextid);
   m_pThread = NULL;
   m_interval = milliseconds;
   m_oneshot = oneshot;
   m_timer = NULL;
}

EThreadBase::Timer::~Timer()
{
   destroy();
}

Void EThreadBase::Timer::init(EThreadBase *pThread)
{
   m_pThread = pThread;

   struct sigevent sev = {};
   sev.sigev_notify = SIGEV_SIGNAL;
   sev.sigev_signo = SIGRTMIN;
   sev.sigev_value.sival_ptr = this;
   if (timer_create(CLOCK_REALTIME, &sev, &m_timer) == -1)
      throw EThreadTimerError_UnableToInitialize();
}

Void EThreadBase::Timer::destroy()
{
   if (m_timer != NULL)
   {
      stop();
      timer_delete(m_timer);
      m_timer = NULL;
   }
}

Void EThreadBase::Timer::start()
{
   if (m_timer == NULL)
      throw EThreadTimerError_NotInitialized();

   struct itimerspec its;
   its.it_value.tv_sec = m_interval / 1000;              // seconds
   its.it_value.tv_nsec = (m_interval % 1000) * 1000000; // nano-seconds
   its.it_interval.tv_sec = m_oneshot ? 0 : its.it_value.tv_sec;
   its.it_interval.tv_nsec = m_oneshot ? 0 : its.it_value.tv_nsec;
   if (timer_settime(m_timer, 0, &its, NULL) == -1)
      throw EThreadTimerError_UnableToStart();
}

Void EThreadBase::Timer::stop()
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

void EThreadBase::Timer::_timerHandler(int signo, siginfo_t *pinfo, void *pcontext)
{
   EThreadBase::Timer *pTimer = (EThreadBase::Timer *)pinfo->si_value.sival_ptr;
   if (pTimer)
      pTimer->m_pThread->sendMessage(EM_TIMER, pTimer);
}

EThreadBase::TimerHandler _initTimerHandler;

Void EThreadBase::TimerHandler::init(EGetOpt &options)
{
   struct sigaction sa;
   sa.sa_flags = SA_SIGINFO;
   sa.sa_sigaction = EThreadBase::Timer::_timerHandler;
   sigemptyset(&sa.sa_mask);
   int signo = SIGRTMIN;
   if (sigaction(signo, &sa, NULL) == -1)
      throw EThreadTimerError_UnableToRegisterTimerHandler();
}

Void EThreadBase::TimerHandler::uninit()
{
}
