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

#ifndef __etbase_h_included
#define __etbase_h_included

#include <time.h>

#include "ebase.h"
#include "eerror.h"
#include "egetopt.h"
#include "estatic.h"
#include "esynch.h"
#include "etq.h"

/// @file

class EThreadBasic;
class EThreadBase;

/// @cond DOXYGEN_EXCLUDE
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
/// @endcond

/// @brief An abstract class that represents contains the threadProc() that
/// will be run in a separate thread.
///
/// @details 
/// EThreadBasic is the base class that represents a single CPU thread.  The
/// function that will be called in the new thread is EThreadBasic::threadProc().
/// Additionally, EThreadBasic is one of the base classes used by the event
/// thread base class EThreadBase.
///
class EThreadBasic
{
   friend class EThreadBase;
   friend class EpcTools;

public:
   /// @brief EThreadBasic run states.
   enum RunState
   {
      /// the thread is waiting to run
      rsWaitingToRun,
      /// the thread is running
      rsRunning,
      /// the threadProc has exited and the thread is no longer running
      rsDoneRunning
   };

   /// @brief Class constructor.
   EThreadBasic();
   /// @brief Class destructor.
   virtual ~EThreadBasic();

   /// @brief Function that will be called in a separate thread.
   ///
   /// @param arg passed by init() to the new thread's threadProc() function
   ///
   /// @details
   /// This pure virtual function must be overridden in the derived class.  The
   /// code in the derived class will be executed in a separate thread.
   ///
   virtual Dword threadProc(pVoid arg) = 0;

   /// @brief Initialize and start the thread.
   ///
   /// @param arg the threadProc() argument
   /// @param stackSize the stack size for the new thread, 0 - default stack size
   ///
   /// @throws EThreadError_AlreadyInitialized the thread has already been initialized
   /// @throws EThreadError_UnableToInitialize unable to initialize the thread
   ///
   /// @details
   /// Initializes and starts the thread with the specified argument and stack
   /// size.  A stackSize of 0 corresponds to the default stack size.
   /// 
   Void init(pVoid arg, size_t stackSize = 0);
   /// @brief Waits for the thread to terminate.
   ///
   /// @throws EThreadError_AlreadyInitialized already initialized
   /// @throws EThreadError_UnableToInitialize unable to initailize
   ///
   /// @details
   /// Suspends the execution of the calling thread until the thread associated
   /// with this object terminates.
   ///
   Void join();

   /// @brief Sleeps for the specified number of milliseconds.
   ///
   /// @details Causes the calling thread to sleep for the specified number of
   /// milliseconds.
   ///
   static Void sleep(Int milliseconds);
   /// @brief Relinquishes the CPU.
   ///
   /// @details
   /// Causes the calling thread to relinquish the CPU.  The thread is moved to
   /// the end of the queue for its static priority and another thread gets to
   /// run.
   ///
   static Void yield();

   /// @brief Returns the thread initialization state.
   ///
   /// @return the initialization status
   ///
   /// @details
   /// Returns True if the thread has been initialized and False if not.
   ///
   Bool isInitialized();

   /// @brief Returns the current thread run state.
   ///
   /// @return the current EThreadBasic::RunState
   ///
   /// @details
   /// Returns the current EThreadBasic::RunState for this thread.
   ///
   RunState getRunState() { return m_state; }

   /// @brief Determines if the thread is waiting to run.
   ///
   /// @return indicates if the thread is waiting to run or not
   ///
   /// @details
   /// Returns True if the thread is in the rsWaitingToRun state and False if
   /// it is not.
   ///
   Bool isWaitingToRun() { return m_state == EThreadBasic::rsWaitingToRun; }
   /// @brief Determines if the thread is running.
   ///
   /// @return indicates if the thread is running or not
   ///
   /// @details
   /// Returns True if the thread is in the rsRunning state and False if
   /// it is not.
   ///
   Bool isRunning() { return m_state == EThreadBasic::rsRunning; }
   /// @brief Determines if the thread has finished running.
   ///
   /// @return indicates if the thread is finished running or not
   ///
   /// @details
   /// Returns True if the thread is in the rsDoneRunning state and False if
   /// it is not.
   ///
   Bool isDoneRunning() { return m_state == EThreadBasic::rsDoneRunning; }

   /// @brief Sends a cancellation request to the thread.
   ///
   /// @return 0 if successful otherwise a nonzero error number.
   ///
   /// @details
   /// Sends a cancellation request to the thread.  Whether and when the target
   /// thread reacts to the cancellation request depends on two attributes that
   /// are under the control of that thread: its cancelability state and type.
   /// See pthread_cancel() for more information.
   ///
   Int cancelWait();

protected:
   /// @brief performs internal initialization *** DO NOT CALL ***
   static Void Initialize();
   /// @brief performs internal de-initialization *** DO NOT CALL ***
   static Void UnInitialize();

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

/// message map declaration macro
#define DECLARE_MESSAGE_MAP()                               \
protected:                                                  \
   static const ethread_msgmap_t *GetThisMessageMap();      \
   virtual const ethread_msgmap_t *GetMessageMap() const;

/// @cond DOXYGEN_EXCLUDE
struct ethread_msgmap_t;
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
/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief base class for EThreadPrivate and EThreadPublic
///
/// @details The EThreadBase class provides event dispatching and timer
/// management functionality that are part of EThreadPrivate and EThreadPublic.
/// the EThreadPrivate and EThreadPublic classes.  This functionality includes
/// event dispatching 
///
class EThreadBase : public _EThreadBase, public EThreadBasic
{
public:
   /// @brief Thread timer class.
   ///
   /// @details EThreadBase::Timer represents an individual timer.  When the
   /// timer expires, the EM_TIMER event will be raised.  The application
   /// can handle the timer by overrideing the onTimer method.
   ///
   class Timer : public EStatic
   {
      friend class EThreadBase;

   /// @cond DOXYGEN_EXCLUDE
   protected:
      Void init(EThreadBase *pThread);
   /// @endcond

   public:
      /// @brief Default class constructor.
      Timer();
      /// @brief Class constructor with configuration parameters.
      ///
      /// @param milliseconds the number of milliseconds before the timer expires
      /// @param oneshot True - one shot timer, False - periodic (recurring) timer
      ///
      Timer(Long milliseconds, Bool oneshot = False);
      /// @brief Class destructor.
      ~Timer();

      /// @brief Stops and destroys the underlying timer object.
      ///
      /// @details Calling destroy() will stop the timer and then delete the
      /// underlying timer object.  This method is called by the destructor.
      ///
      Void destroy();
      /// @brief Starts the timer.
      ///
      /// @throws EThreadTimerError_NotInitialized timer not initialized
      /// @throws EThreadTimerError_UnableToStart unable to start the timer
      /// 
      Void start();
      /// @brief Stops the timer.
      Void stop();

      /// @brief Returns the timer interval in milliseconds.
      Long getInterval() { return m_interval; }
      /// @brief sets the timer interval
      ///
      /// @param interval the timer interval in milliseconds.
      ///
      Void setInterval(Long interval) { m_interval = interval; }
      /// @brief sets the type of timer
      ///
      /// @param oneshot True - one shot timer, False - periodic (recurring timer)
      ///
      Void setOneShot(Bool oneshot) { m_oneshot = oneshot; }
      /// @brief Returns the unique timer id.
      ///
      /// @details
      /// The timer ID is created internally when the timer object is
      /// instantiated.
      ///
      Long getId() { return m_id; }
      /// @brief Indicates if this timer object has been initialized.
      ///
      /// @details
      /// The timer ID is created internally when the timer object is
      /// instantiated.
      ///
      Bool isInitialized() { return m_timer != NULL; }

   private:
      static Long m_nextid;

      Long m_id;
      EThreadBase *m_pThread;
      Bool m_oneshot;
      Long m_interval;
      timer_t m_timer;
      static void _timerHandler(int signo, siginfo_t *pinfo, void *pcontext);
   };

   /// @cond DOXYGEN_EXCLUDE
   class TimerHandler : EStatic
   {
   public:
      TimerHandler() {}
      ~TimerHandler() {}

      virtual Int getInitType() { return STATIC_INIT_TYPE_THREADS; }
      Void init(EGetOpt &options);
      Void uninit();
   };
   /// @endcond

public:
   /// @brief Default class constructor.
   EThreadBase();
   /// @brief Rhe class destructor.
   ~EThreadBase();

   /// @brief Sends event message to this thread.
   ///
   /// @param message the message ID
   /// @param wait waits for the message to be sent
   ///
   /// @details
   /// Sends (posts) an event message to this threads event queue.  No
   /// additional data is posted with the event.
   /// 
   Bool sendMessage(UInt message, Bool wait = True);
   /// @brief Sends event message to this thread.
   ///
   /// @param message the message ID
   /// @param lowPart 32-bit value to include with the message
   /// @param highPart 32-bit value to include with the message
   /// @param wait waits for the message to be sent
   ///
   /// @details
   /// Sends (posts) an event message to this threads event queue.
   /// Two 32-bit values are included with the message.
   /// 
   Bool sendMessage(UInt message, Dword lowPart, Long highPart, Bool wait = True);
   /// @brief Sends event message to this thread.
   ///
   /// @param message the message ID
   /// @param voidPtr a Void* to include with the message
   /// @param wait waits for the message to be sent
   ///
   /// @details
   /// Sends (posts) an event message to this threads event queue.
   /// One 64-bit Void* value is included with the message.
   /// 
   Bool sendMessage(UInt message, pVoid voidPtr, Bool wait = True);
   /// @brief Sends event message to this thread.
   ///
   /// @param message the message ID
   /// @param quadPart a 64-bit value to include with the message
   /// @param wait waits for the message to be sent
   ///
   /// @details
   /// Sends (posts) an event message to this threads event queue.
   /// One 64-bit value is included with the message.
   /// 
   Bool sendMessage(UInt message, LongLong quadPart, Bool wait = True);

   /// @brief Initializes the EThreadBase thread object.
   ///
   /// @param arg the argument that is passed to EThreadBasic::init()
   /// @param suspended indicates whether to suspend the thread at initialization
   /// @param stackSize the stack size for the new thread, 0 - default stack size
   ///
   Void init(pVoid arg, Bool suspended = False, size_t stackSize = 0);
   /// @brief Posts the quit message to this thread.
   Void quit();
   /// @brief Initializes the thread when it was suspended at init().
   Void start();
   /// @brief Suspends a running thread.
   ///
   /// @details
   /// Suspends a running thread by posting the EM_SUSPEND message to the
   /// threads event queue.  The thread will call the onSuspend() method
   /// when the EM_SUSPEND event is processed.  The thread will not process
   /// any more thread messages until the resume() method is called.
   ///
   Void suspend();
   /// @brief Resumes a suspended thread.
   Void resume();

   /// @brief Returns the semaphore associated with this thread's event queue.
   ESemaphoreData &getMsgSemaphore()
   {
      return queue().semMsgs();
   }

   /// @brief Called in the context of the thread when the EM_INIT event is processed.
   virtual Void onInit();
   /// @brief Called in the context of the thread when the EM_QUIT event is processed.
   virtual Void onQuit();
   /// @brief Called in the context of the thread when th EM_SUSPEND event is processed.
   virtual Void onSuspend();
   /// @brief Called in the context of the thread when th EM_TIMER event is processed.
   ///
   /// @param ptimer a pointer to the EThreadBase::Timer object that expired
   ///
   virtual Void onTimer(EThreadBase::Timer *ptimer);

   /// @brief Intializes an EThreadBase::Timer object and associates with this thread.
   ///
   /// @param t the EThreadBase::Timer object to initialize
   Void initTimer(Timer &t)
   {
      t.init(this);
   }

   DECLARE_MESSAGE_MAP()

protected:
   /// @cond DOXYGEN_EXCLUDE
   virtual EThreadQueueBase &queue() = 0;
   /// @endcond

   /// @brief Called when an event message is queued.
   ///
   /// @details
   /// This method is called in the context of the thread where sendMessage()
   /// is called after the message has been added to the thread's event queue.
   virtual Void messageQueued();
   /// @brief Dispatches the next thread event message.
   ///
   /// @param msg the EThreadMessage event object
   /// @param wait waits for the next EThreadMessage to be processed
   ///
   /// @details
   /// This method retrieves the next event message from the threads event
   /// queue and processes the event by calling the first event handler as
   /// defined by the class heirarchy.  If no event message is available and
   /// the wait parameter is true, then the thread will block waiting on the
   /// next event message to be sent to the thread.  If there is no event
   /// handler defined in the class heirarchy for a particular event ID, the
   /// default event handler, defMessageHandler(), will be called.
   ///
   Bool pumpMessage(EThreadMessage &msg, Bool wait = true);
   /// @brief Process event messages.
   ///
   /// @throws EError catches and re-throws any exception raised by pumpMessage
   ///
   /// @details
   /// Any overridden version of pumpMessages() must call pumpMessage() to
   /// process each individual message.
   ///
   virtual Void pumpMessages();
   /// @brief The default event message handler.
   ///
   /// @param msg the EThreadMessage event object
   ///
   /// This method is called when no event handler has been defined in the
   /// class heirarchy for a specified thread event.
   ///
   virtual Void defMessageHandler(EThreadMessage &msg);

private:
   Dword threadProc(pVoid arg);
   Bool dispatch(EThreadMessage &msg);

   pVoid m_arg;
   size_t m_stacksize;
   Int m_suspendCnt;
   ESemaphorePrivate m_suspendSem;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE
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
/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// thread initialization event
#define EM_INIT 1
/// thread quit event
#define EM_QUIT 2
/// thread suspend event
#define EM_SUSPEND 3
/// thread timer expiration event
#define EM_TIMER 4
/// socket read event see ESocketThread
#define EM_SOCKETSELECT_READ 5
/// socket write event see ESocketThread
#define EM_SOCKETSELECT_WRITE 6
/// socket select error event see ESocketThread
#define EM_SOCKETSELECT_ERROR 7
/// socket exception event see ESocketThread
#define EM_SOCKETSELECT_EXCEPTION 8
/// beginning of user events
#define EM_USER 10000

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// starts the message map declaration
#define BEGIN_MESSAGE_MAP(theClass, baseClass)              \
   const ethread_msgmap_t *theClass::GetMessageMap() const  \
   {                                                        \
      return GetThisMessageMap();                           \
   }                                                        \
   const ethread_msgmap_t *theClass::GetThisMessageMap()    \
   {                                                        \
      typedef theClass ThisClass;                           \
      typedef baseClass TheBaseClass;                       \
      static const ethread_msgentry_t _msgEntries[] =       \
      {

/// starts the message map declaration
#define END_MESSAGE_MAP()                                   \
         {0, eMsgFxnEnd, (ethread_msgfxnA_t)0}              \
      };                                                    \
   static const ethread_msgmap_t msgMap =                   \
       {&TheBaseClass::GetThisMessageMap, &_msgEntries[0]}; \
   return &msgMap;                                          \
   }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// adds EM_INIT event handler
#define ON_EM_INIT() \
         {EM_INIT, eMsgFxnA, (ethread_msgfxnA_t)(&EThreadBase::onInit)},

/// adds EM_QUIT event handler
#define ON_EM_QUIT() \
         {EM_QUIT, eMsgFxnA, (ethread_msgfxnA_t)(&EThreadBase::onQuit)},

/// adds EM_SUSPEND event handler
#define ON_EM_SUSPEND() \
         {EM_SUSPEND, eMsgFxnA, (ethread_msgfxnA_t)(&EThreadBase::onSuspend)},

/// adds EM_TIMER event handler
#define ON_EM_TIMER() \
         {EM_TIMER, eMsgFxnD, (ethread_msgfxnA_t)(&EThreadBase::onTimer)},

/// adds user defined event handler
#define ON_MESSAGE(id, memberFxn) \
         {id, eMsgFxnB, (ethread_msgfxnA_t)(Void(EThreadBase::*)(EThreadMessage &))(&memberFxn)},

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif // #define __etbase_h_included
