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

#ifndef __etbase_h_included
#define __etbase_h_included

#include <time.h>

#include "ebase.h"
#include "eerror.h"
#include "egetopt.h"
#include "estatic.h"
#include "esynch.h"
#include "etq.h"

class EThreadBasic;
class EThreadBase;

class EThreadError_InvalidHandle : public EError
{
};
class EThreadError_NotInitialized : public EError
{
};
class EThreadError_AlreadyInitialized : public EError
{
};
class EThreadError_UnableToResume : public EError
{
public:
   EThreadError_UnableToResume();
};
class EThreadError_UnableToSuspend : public EError
{
public:
   EThreadError_UnableToSuspend();
};
class EThreadError_UnableToInitialize : public EError
{
public:
   EThreadError_UnableToInitialize();
};

DECLARE_ERROR_ADVANCED(EThreadTimerError_UnableToRegisterTimerHandler);
DECLARE_ERROR_ADVANCED(EThreadTimerError_UnableToInitialize);
DECLARE_ERROR_ADVANCED(EThreadTimerError_NotInitialized);
DECLARE_ERROR_ADVANCED(EThreadTimerError_UnableToStart);
DECLARE_ERROR_ADVANCED(EThreadTimerError_UnableToStop);

typedef list<EThreadBasic *> EThreadPtrList;

class EThreadBasic
{
   friend class EThreadBase;

public:
   enum RunState
   {
      rsWaitingToRun,
      rsRunning,
      rsDoneRunning
   };

   EThreadBasic();
   virtual ~EThreadBasic();

   virtual Dword threadProc(pVoid arg) = 0;

   static Void Initialize();
   static Void UnInitialize();

   Void init(pVoid arg, Dword stackSize = 0);
   Void join();

   static Void sleep(Int milliseconds);
   static Void yield();

   Bool isInitialized();

   Void setRunState(RunState state) { m_state = state; }
   RunState getRunState() { return m_state; }

   Bool isWaitingToRun() { return m_state == EThreadBasic::rsWaitingToRun; }
   Bool isRunning() { return m_state == EThreadBasic::rsRunning; }
   Bool isDoneRunning() { return m_state == EThreadBasic::rsDoneRunning; }

   Int cancelWait();

private:
   Bool m_initialized;
   pthread_t m_thread;
   static pVoid _threadProc(pVoid arg);
   static EThreadBasic *findCurrentThread();
   static EThreadPtrList m_thrdCtl;
   static EMutexPrivate m_thrdCtlMutex;

   Void _shutdown();

   EMutexPrivate m_mutex;
   RunState m_state;
   pVoid m_arg;
   Dword m_exitCode;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct ethread_msgmap_t;

#define DECLARE_MESSAGE_MAP()                           \
protected:                                              \
   static const ethread_msgmap_t *GetThisMessageMap(); \
   virtual const ethread_msgmap_t *GetMessageMap() const;

class _EThreadBase
{
   friend class EThreadBase;

protected:
   virtual const ethread_msgmap_t *GetMessageMap() const
   {
      return GetThisMessageMap();
   }

   static const ethread_msgmap_t *GetThisMessageMap()
   {
      return NULL;
   }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EThreadBase : public _EThreadBase, public EThreadBasic
{
public:
   class Timer : public EStatic
   {
      friend class EThreadBase;

   protected:
      Void init(EThreadBase *pThread);

   public:
      Timer();
      Timer(Long milliseconds, Bool oneshot = False);
      ~Timer();

      Void destroy();
      Void start();
      Void stop();

      Long getInterval() { return m_interval; }
      Void setInterval(Long interval) { m_interval = interval; }
      Void setOneShot(Bool oneshot) { m_oneshot = oneshot; }
      Long getId() { return m_id; }

   private:
      static Long m_nextid;

      Long m_id;
      EThreadBase *m_pThread;
      Bool m_oneshot;
      Long m_interval;
      timer_t m_timer;
      static void _timerHandler(int signo, siginfo_t *pinfo, void *pcontext);
   };

   class TimerHandler : EStatic
   {
   public:
      TimerHandler() {}
      ~TimerHandler() {}

      virtual Int getInitType() { return STATIC_INIT_TYPE_THREADS; }
      Void init(EGetOpt &options);
      Void uninit();
   };

public:
   EThreadBase();
   ~EThreadBase();

   Bool sendMessage(UInt message, Bool wait = True);
   Bool sendMessage(UInt message, Dword lowPart, Long highPart, Bool wait = True);
   Bool sendMessage(UInt message, pVoid voidPtr, Bool wait = True);
   Bool sendMessage(UInt message, LongLong quadPart, Bool wait = True);

   Void init(pVoid arg, Bool suspended = False, Dword stackSize = 0);
   Void quit();
   Void start();
   Void suspend();
   Void resume();
   Bool pumpMessage(EThreadMessage &msg, Bool wait = true);

   ESemaphoreData &getMsgSemaphore()
   {
      return queue().semMsgs();
   }

   virtual Void onInit();
   virtual Void onQuit();
   virtual Void onSuspend();
   virtual Void onTimer(EThreadBase::Timer *ptimer);

   virtual Void pumpMessages();
   virtual Void defMessageHandler(EThreadMessage &msg);

   Void initTimer(Timer &t)
   {
      t.init(this);
   }

   DECLARE_MESSAGE_MAP()

protected:
   virtual EThreadQueueBase &queue() = 0;
   virtual Void messageQueued();

private:
   Dword threadProc(pVoid arg);
   Bool dispatch(EThreadMessage &msg);

   pVoid m_arg;
   Dword m_stacksize;
   Int m_suspendCnt;
   ESemaphorePrivate m_suspendSem;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef Void (EThreadBase::*ethread_msgfxnA_t)();
typedef Void (EThreadBase::*ethread_msgfxnB_t)(EThreadMessage &);
typedef Void (EThreadBase::*ethread_msgfxnC_t)(Long);
typedef Void (EThreadBase::*ethread_msgfxnD_t)(pVoid);

enum ethread_msgfnx_e
{
   eMsgFxnEnd,
   eMsgFxnA,
   eMsgFxnB,
   eMsgFxnC,
   eMsgFxnD
};

typedef struct
{
   UInt nMessage;             // message
   ethread_msgfnx_e nFnType; // function type
   ethread_msgfxnA_t pFn;    // routine to call (or special value)
} ethread_msgentry_t;

struct ethread_msgmap_t
{
   const ethread_msgmap_t *(*pfnGetBaseMap)();
   const ethread_msgentry_t *lpEntries;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define EM_INIT 1
#define EM_QUIT 2
#define EM_SUSPEND 3
#define EM_TIMER 4
#define EM_SOCKETSELECT_READ 5
#define EM_SOCKETSELECT_WRITE 6
#define EM_SOCKETSELECT_ERROR 7
#define EM_SOCKETSELECT_EXCEPTION 8
#define EM_USER 10000

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define BEGIN_MESSAGE_MAP(theClass, baseClass)              \
   const ethread_msgmap_t *theClass::GetMessageMap() const \
   {                                                        \
      return GetThisMessageMap();                           \
   }                                                        \
   const ethread_msgmap_t *theClass::GetThisMessageMap()   \
   {                                                        \
      typedef theClass ThisClass;                           \
      typedef baseClass TheBaseClass;                       \
      static const ethread_msgentry_t _msgEntries[] =      \
      {

#define END_MESSAGE_MAP()                                   \
   {                                                        \
      0, eMsgFxnEnd, (ethread_msgfxnA_t)0                  \
   }                                                        \
   }                                                        \
   ;                                                        \
   static const ethread_msgmap_t msgMap =                  \
       {&TheBaseClass::GetThisMessageMap, &_msgEntries[0]}; \
   return &msgMap;                                          \
   }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define ON_EM_INIT() \
   {EM_INIT, eMsgFxnA, (ethread_msgfxnA_t)(&EThreadBase::onInit)},

#define ON_EM_QUIT() \
   {EM_QUIT, eMsgFxnA, (ethread_msgfxnA_t)(&EThreadBase::onQuit)},

#define ON_EM_SUSPEND() \
   {EM_SUSPEND, eMsgFxnA, (ethread_msgfxnA_t)(&EThreadBase::onSuspend)},

#define ON_EM_TIMER() \
   {EM_TIMER, eMsgFxnD, (ethread_msgfxnA_t)(&EThreadBase::onTimer)},

#define ON_MESSAGE(id, memberFxn) \
   {id, eMsgFxnB, (ethread_msgfxnA_t)(Void(EThreadBase::*)(EThreadMessage &))(&memberFxn)},

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif // #define __etbase_h_included
