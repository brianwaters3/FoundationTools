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

#include "eatomic.h"
#include "etbasic.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

EThreadError_UnableToInitialize::EThreadError_UnableToInitialize() : EError()
{
   setSevere();
   setText("Error initializing thread ");
   appendLastOsError();
}

/// @endcond

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

Void EThreadBasic::sleep(Int milliseconds)
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
