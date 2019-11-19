/*
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

#ifndef __ETBASIC_H
#define __ETBASIC_H

#include "ebase.h"
#include "eerror.h"
#include "esynch.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EThreadError_AlreadyInitialized : public EError
{
};

class EThreadError_UnableToInitialize : public EError
{
public:
   EThreadError_UnableToInitialize();
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EThreadBasic;

typedef list<EThreadBasic *> EThreadPtrList;

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

   /// @brief
   Void signal(Int sig) { pthread_kill(m_thread, sig); }

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

#endif // #ifndef __ETBASIC_H