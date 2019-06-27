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

#include <errno.h>

#include "ftinternal.h"
#include "ftsynch.h"
#include "ftsynch2.h"
#include "ftthread.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTMutexError_UnableToInitialize::FTMutexError_UnableToInitialize(Int err)
{
   setSevere();
   setText("Unable to initialize mutex ");
   appendLastOsError(err);
}

FTMutexError_UnableToLock::FTMutexError_UnableToLock(Int err)
{
   setSevere();
   setText("Unable to lock mutex ");
   appendLastOsError(err);
}

FTMutexError_UnableToUnLock::FTMutexError_UnableToUnLock(Int err)
{
   setSevere();
   setText("Unable to unlock mutex ");
   appendLastOsError(err);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTSemaphoreError_UnableToInitialize::FTSemaphoreError_UnableToInitialize()
{
   setSevere();
   setText("Error creating semaphore ");
   appendLastOsError();
}

FTSemaphoreError_UnableToDecrement::FTSemaphoreError_UnableToDecrement()
{
   setSevere();
   setText("Error decrementing semaphore ");
   appendLastOsError();
}

FTSemaphoreError_UnableToIncrement::FTSemaphoreError_UnableToIncrement()
{
   setSevere();
   setText("Error incrementing semaphore ");
   appendLastOsError();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTSynchObjectsError_UnableToAllocateSynchObject::FTSynchObjectsError_UnableToAllocateSynchObject(Int err)
{
   setSevere();
   setText("Unable to allocate synch object ");
   appendLastOsError(err);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTSynchObjects _synchObjCtrl;
FTSynchObjects *FTSynchObjects::m_pThis = NULL;

Void FTSynchObjects::FTSynchObjectsSharedMemory::onDestroy()
{
}

Void FTSynchObjects::FTSynchObjectsSharedMemory::setSynchObjectsPtr(FTSynchObjects *p)
{
   m_pSynchObjects = p;
}

FTSynchObjects::FTSynchObjects()
{
   m_pCtrl = NULL;
   m_pSemaphores = NULL;
   m_pMutexes = NULL;
}

FTSynchObjects::~FTSynchObjects()
{
}

Void FTSynchObjects::init(FTGetOpt &options)
{
   options.setPrefix(SECTION_TOOLS);
   if (options.get(MEMBER_DEBUG, false))
      options.print();
   if (!options.get(MEMBER_ENABLE_PUBLIC_OBJECTS, false))
      return;
   options.setPrefix("");

   options.setPrefix(SECTION_TOOLS "/" SECTION_SYNCH_OBJS);
   Int nSemaphores = options.get(MEMBER_NUMBER_SEMAPHORES, 0);
   Int nMutexes = options.get(MEMBER_NUMBER_MUTEXES, 0);
   options.setPrefix("");

   options.setPrefix(SECTION_TOOLS);
   UInt nPublicQueues = options.getCount(SECTION_PUBLIC_QUEUE);
   options.setPrefix("");

   m_sharedmem.init("FTSynchObjectPublicStorage", 'A',
                    sizeof(Bool) +             // initialization flag
                    sizeof(ftsynchcontrol_t) + // synch control block
                    //sizeof(ftsynchcontrol_t) + // semaphore control block
                    //sizeof(ftsynchcontrol_t) + // mutex control block
                    //sizeof(ftsynchcontrol_t) + // condition variable control block
                    sizeof(FTSemaphoreDataPublic) * nSemaphores +
                    sizeof(FTMutexDataPublic) * nMutexes +                    // storage for mutex objects
                    sizeof(ftpublicqueuedef_t) * (nPublicQueues + 1) // storage for public queue definitions
   );

   m_pCtrl = (ftsynchcontrol_t *)m_sharedmem.getDataPtr();

   m_pSemaphores = (FTSemaphoreDataPublic *)((pChar)m_pCtrl + sizeof(*m_pCtrl));
   m_pMutexes = (FTMutexDataPublic *)((pChar)m_pSemaphores + (sizeof(FTSemaphoreDataPublic) * nSemaphores));
   m_pPubQueues = (ftpublicqueuedef_t *)((pChar)m_pMutexes + (sizeof(FTMutexDataPublic) * nMutexes));

   if (!m_pCtrl->m_initialized)
   {
      m_pCtrl->m_sequence = 0;

      new(&m_pCtrl->m_semaphoreCtrl.m_mutex) FTMutexPrivate();
      new(&m_pCtrl->m_mutexCtrl.m_mutex) FTMutexPrivate();

      FTMutexLock ls(m_pCtrl->m_semaphoreCtrl.m_mutex);
      FTMutexLock lm(m_pCtrl->m_mutexCtrl.m_mutex);

      m_pCtrl->m_semaphoreCtrl.m_max = nSemaphores;
      m_pCtrl->m_semaphoreCtrl.m_head = 0;
      m_pCtrl->m_semaphoreCtrl.m_currused = 0;
      m_pCtrl->m_semaphoreCtrl.m_maxused = 0;

      m_pCtrl->m_mutexCtrl.m_max = nMutexes;
      m_pCtrl->m_mutexCtrl.m_head = 0;
      m_pCtrl->m_mutexCtrl.m_currused = 0;
      m_pCtrl->m_mutexCtrl.m_maxused = 0;

      Int ofs;

      for (ofs = 1; ofs < m_pCtrl->m_semaphoreCtrl.m_max; ofs++)
      {
         m_pSemaphores[ofs - 1].semIndex() = ofs;
         m_pSemaphores[ofs - 1].nextIndex() = ofs;
      }
      m_pSemaphores[ofs - 1].semIndex() = ofs;
      m_pSemaphores[ofs - 1].nextIndex() = -1;

      for (ofs = 1; ofs < m_pCtrl->m_mutexCtrl.m_max; ofs++)
      {
         m_pMutexes[ofs - 1].mutexId() = ofs; //setMutexId(ofs);
         m_pMutexes[ofs - 1].nextIndex() = ofs; //setNextIndex(ofs);
      }
      m_pMutexes[ofs - 1].mutexId() = ofs; //setMutexId(ofs);
      m_pMutexes[ofs - 1].nextIndex() = ofs; //setNextIndex(-1);

      memset(m_pPubQueues, 0, sizeof(ftpublicqueuedef_t) * (nPublicQueues + 1));

      m_pCtrl->m_initialized = True;

      m_pThis = this;
   }

   ////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////

   FTString s;
   options.setPrefix(SECTION_TOOLS);
   for (UInt idx = 0; idx < nPublicQueues; idx++)
   {
      Int queueid = options.get(idx, SECTION_PUBLIC_QUEUE, MEMBER_QUEUE_ID, -1);
      Int msgsize = options.get(idx, SECTION_PUBLIC_QUEUE, MEMBER_MESSAGE_SIZE, -1);
      Int queuesize = options.get(idx, SECTION_PUBLIC_QUEUE, MEMBER_QUEUE_SIZE, -1);
      Bool bR = options.get(idx, SECTION_PUBLIC_QUEUE, MEMBER_ALLOW_MULTIPLE_READERS, false);
      Bool bW = options.get(idx, SECTION_PUBLIC_QUEUE, MEMBER_ALLOW_MULTIPLE_WRITERS, false);

      s.format(SECTION_PUBLIC_QUEUE "%u", idx);
      setPublicQueue(idx, s.c_str(),
                     queueid, msgsize, queuesize, bR, bW);

      //cout << (*it + "QueueID = ") << o((*it + "QueueID").c_str(), "Unknown QueueID") << endl;
      //cout << (*it + "MessageSize = ") << o((*it + "MessageSize").c_str(), "Unknown MessageSize") << endl;
      //cout << (*it + "QueueSize = ") << o((*it + "QueueSize").c_str(), "Unknown QueueSize") << endl;
      //cout << (*it + "AllowMultipleReaders = ") << o((*it + "AllowMultipleReaders").c_str(), "Unknown AllowMultipleReaders") << endl;
      //cout << (*it + "AllowMultipleWriters = ") << o((*it + "AllowMultipleWriters").c_str(), "Unknown AllowMultipleWriters") << endl;
   }
   options.setPrefix("");
}

Void FTSynchObjects::uninit()
{
   // there should be no need to "uninit" since
   // if all users are done, the shared memory block
   // will just be thrown away (and all of the "uninit'ing"
   // will occur in the shared memory
   //
}

Void FTSynchObjects::logObjectUsage()
{
   // FTLOGINFO(FTLOG_SYNCHOBJECTS, "Maximum number of semaphores allocated was %ld", m_pCtrl->m_semaphoreCtrl.m_maxused);
   // FTLOGINFO(FTLOG_SYNCHOBJECTS, "Maximum number of mutexes allocated was %ld", m_pCtrl->m_mutexCtrl.m_maxused);
}

Int FTSynchObjects::nextSemaphore()
{
   FTSynchObjects *pso = getSynchObjCtrlPtr();
   ftsynchcontrol_t *pctrl = pso->m_pCtrl;
   
   FTMutexLock l(pctrl->m_semaphoreCtrl.m_mutex);

   if (pctrl->m_semaphoreCtrl.m_head == -1)
      throw FTSemaphoreError_UnableToAllocateSemaphore();

   Int ofs = pctrl->m_semaphoreCtrl.m_head;
   pctrl->m_semaphoreCtrl.m_head =
       pso->m_pSemaphores[pctrl->m_semaphoreCtrl.m_head].nextIndex();

   pctrl->m_semaphoreCtrl.m_currused++;
   if (pctrl->m_semaphoreCtrl.m_currused > pctrl->m_semaphoreCtrl.m_maxused)
      pctrl->m_semaphoreCtrl.m_maxused = pctrl->m_semaphoreCtrl.m_currused;

   return ofs + 1;
}

Int FTSynchObjects::nextMutex()
{
   FTSynchObjects *pso = getSynchObjCtrlPtr();
   ftsynchcontrol_t *pctrl = pso->m_pCtrl;
   
   FTMutexLock l(pctrl->m_mutexCtrl.m_mutex);

   if (pctrl->m_mutexCtrl.m_head == -1)
      throw FTMutexError_UnableToAllocateMutex();

   Int ofs = pctrl->m_mutexCtrl.m_head;
   pctrl->m_mutexCtrl.m_head =
       pso->m_pMutexes[pctrl->m_mutexCtrl.m_head].nextIndex();

   pctrl->m_mutexCtrl.m_currused++;
   if (pctrl->m_mutexCtrl.m_currused > pctrl->m_mutexCtrl.m_maxused)
      pctrl->m_mutexCtrl.m_maxused = pctrl->m_mutexCtrl.m_currused;

   return ofs + 1;
}

Void FTSynchObjects::freeSemaphore(Int nSemId)
{
   FTSynchObjects *pso = getSynchObjCtrlPtr();
   ftsynchcontrol_t *pctrl = pso->m_pCtrl;
   
   FTMutexLock l(pctrl->m_semaphoreCtrl.m_mutex);

   nSemId--;

   pso->m_pSemaphores[nSemId].nextIndex() = pctrl->m_semaphoreCtrl.m_head;
   pctrl->m_semaphoreCtrl.m_head = nSemId;

   pctrl->m_semaphoreCtrl.m_currused--;
}

Void FTSynchObjects::freeMutex(Int nMutexId)
{
   FTSynchObjects *pso = getSynchObjCtrlPtr();
   ftsynchcontrol_t *pctrl = pso->m_pCtrl;
   
   FTMutexLock l(pctrl->m_mutexCtrl.m_mutex);

   nMutexId--;

   pso->m_pMutexes[nMutexId].nextIndex() = pctrl->m_mutexCtrl.m_head;
   pctrl->m_mutexCtrl.m_head = nMutexId;

   pctrl->m_mutexCtrl.m_currused--;
}

////////////////////////////////////////////////////////////////////////////////
// Common Mutex Classes
////////////////////////////////////////////////////////////////////////////////

Void FTMutexData::init(Bool isPublic)
{
   if (!m_initialized)
   {
#if defined(NATIVE_IPC)
      Int res;
      pthread_mutexattr_t attr;

      if ((res = pthread_mutexattr_init(&attr)) != 0)
         throw FTMutexError_UnableToInitialize(res);
      if (isPublic && (res = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) != 0)
         throw FTMutexError_UnableToInitialize(res);
      if ((res = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) != 0)
         throw FTMutexError_UnableToInitialize(res);
      if ((res = pthread_mutex_init(&m_mutex, &attr)) != 0)
         throw FTMutexError_UnableToInitialize(res);
#else
      m_lock = 0;
#endif
      m_initialized = True;
   }
}

Void FTMutexData::destroy()
{
   if (m_initialized)
   {
#if defined(NATIVE_IPC)
      pthread_mutex_destroy(&m_mutex);
#else
      m_lock = 0;
#endif
      m_initialized = False;
   }      
}

Bool FTMutexData::enter(Bool wait)
{
#if defined(NATIVE_IPC)
   Int res = pthread_mutex_lock(&data.mutex());
   if (res != 0 && wait)
      throw FTMutexError_UnableToLock(res);
   return res == 0;
#else
   while (True)
   {
      if (atomic_cas(mutex(), 0, 1) == 0)
         return True;
      if (wait)
         FTThreadBasic::yield();
      else
         return False;
   }
#endif
}

Void FTMutexData::leave()
{
#if defined(NATIVE_IPC)
   Int res = pthread_mutex_unlock(&mutex());
   if (res != 0)
      throw FTMutexError_UnableToUnLock(res);
#else
   atomic_swap(mutex(), 0);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Public Mutex Classes
////////////////////////////////////////////////////////////////////////////////

Void FTMutexPublic::init()
{
   if (m_mutexid != 0)
      throw FTMutexError_UnableToAllocateMutex();

   m_mutexid = FTSynchObjects::getSynchObjCtrlPtr()->nextMutex();
   ((FTMutexDataPublic)*this).init(True);
}

Void FTMutexPublic::destroy()
{
   if (m_mutexid == 0)
      return;

   ((FTMutexDataPublic)*this).destroy();
   FTSynchObjects::getSynchObjCtrlPtr()->freeMutex(m_mutexid);
   m_mutexid = 0;
}

FTMutexPublic::operator FTMutexDataPublic &()
{
   if (m_mutexid == 0)
      throw FTMutexError_MutexUninitialized();
   return FTSynchObjects::getSynchObjCtrlPtr()->getMutex(m_mutexid);
}

Void FTMutexPublic::attach(Int mutexid)
{
   if (m_mutexid)
      throw FTMutexError_AlreadyInitialized();
   m_mutexid = mutexid;
}

Void FTMutexPublic::detach()
{
   if (m_mutexid == 0)
      throw FTMutexError_MutexUninitialized();
   m_mutexid = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Common Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

Void FTSemaphoreData::init()
{
   Int res;

   res = sem_init(&m_sem, shared() ? 1 : 0, 0);
   if (res == -1)
      throw FTSemaphoreError_UnableToInitialize();
   
   m_currCount = m_initCount;
   m_initialized = True;
}

Void FTSemaphoreData::destroy()
{
   if (m_initialized)
   {
      shared() = False;
      initialCount() = 0;
      m_currCount = 0;

      sem_destroy(&m_sem);

      m_initialized = False;
   }
}

Bool FTSemaphoreData::Decrement(Bool wait)
{
   if (!initialized())
      throw FTSemaphoreError_NotInitialized();

   Long val = atomic_dec(m_currCount);
   if (val < 0)
   {
      if (wait)
      {
         if (sem_wait(&m_sem) != 0)
         {
            atomic_inc(m_currCount);
            return False;
         }
      }
      else
      {
         atomic_inc(m_currCount);
         return False;
      }
   }
   return True;
}

Bool FTSemaphoreData::Increment()
{
   if (!initialized())
      throw FTSemaphoreError_NotInitialized();

   Long val = atomic_inc(m_currCount);
   if (val < 1)
   {
      if (sem_post(&m_sem) != 0)
      {
         atomic_dec(m_currCount);
         throw FTSemaphoreError_UnableToIncrement();
      }
   }
   return True;
}

////////////////////////////////////////////////////////////////////////////////
// Public Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

FTSemaphoreData &FTSemaphorePublic::getData()
{
   return FTSynchObjects::getSemaphore(m_semid);
}

Void FTSemaphorePublic::init(Long initialCount)
{
   if (m_semid != 0)
      throw FTSemaphoreError_AlreadyInitialized();

   m_semid = FTSynchObjects::getSynchObjCtrlPtr()->nextSemaphore();
   FTSemaphoreDataPublic &d = FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid);
   d.shared() = True;
   d.initialCount() = initialCount;
   d.init();
}

Void FTSemaphorePublic::destroy()
{
   if (m_semid == 0)
      return;
   
   FTSemaphoreDataPublic &d = FTSynchObjects::getSemaphore(m_semid);

   d.destroy();
   FTSynchObjects::freeSemaphore(m_semid);
   m_semid = 0;
}

Int &FTSemaphorePublic::nextIndex()
{
   if (m_semid == 0)
      throw FTSemaphoreError_NotInitialized();
   return FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).nextIndex();      
}

Int &FTSemaphorePublic::semIndex()
{
   if (m_semid == 0)
      throw FTSemaphoreError_NotInitialized();
   return FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).semIndex();      
}

Void FTSemaphorePublic::attach(Int semid)
{
   if (m_semid)
      throw FTSemaphoreError_AlreadyInitialized();
   m_semid = semid;
}

Void FTSemaphorePublic::detach()
{
   if (m_semid == 0)
      throw FTSemaphoreError_NotInitialized();
   m_semid = 0;
}
