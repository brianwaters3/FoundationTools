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

#include <poll.h>
#include <errno.h>

#include "einternal.h"
#include "esynch.h"
#include "esynch2.h"
#include "etevent.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

EMutexError_UnableToInitialize::EMutexError_UnableToInitialize(Int err)
{
   setSevere();
   setText("Unable to initialize mutex ");
   appendLastOsError(err);
}

EMutexError_UnableToLock::EMutexError_UnableToLock(Int err)
{
   setSevere();
   setText("Unable to lock mutex ");
   appendLastOsError(err);
}

EMutexError_UnableToUnLock::EMutexError_UnableToUnLock(Int err)
{
   setSevere();
   setText("Unable to unlock mutex ");
   appendLastOsError(err);
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

ESemaphoreError_UnableToInitialize::ESemaphoreError_UnableToInitialize()
{
   setSevere();
   setText("Error creating semaphore ");
   appendLastOsError();
}

ESemaphoreError_UnableToDecrement::ESemaphoreError_UnableToDecrement()
{
   setSevere();
   setText("Error decrementing semaphore ");
   appendLastOsError();
}

ESemaphoreError_UnableToIncrement::ESemaphoreError_UnableToIncrement()
{
   setSevere();
   setText("Error incrementing semaphore ");
   appendLastOsError();
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

ESynchObjectsError_UnableToAllocateSynchObject::ESynchObjectsError_UnableToAllocateSynchObject(Int err)
{
   setSevere();
   setText("Unable to allocate synch object ");
   appendLastOsError(err);
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE
ESynchObjects _synchObjCtrl;
ESynchObjects *ESynchObjects::m_pThis = NULL;
/// @endcond

Void ESynchObjects::ESynchObjectsSharedMemory::onDestroy()
{
}

Void ESynchObjects::ESynchObjectsSharedMemory::setSynchObjectsPtr(ESynchObjects *p)
{
   m_pSynchObjects = p;
}

/// @cond DOXYGEN_EXCLUDE
ESynchObjects::ESynchObjects()
{
   m_pCtrl = NULL;
   m_pSemaphores = NULL;
   m_pMutexes = NULL;
}

ESynchObjects::~ESynchObjects()
{
}

Void ESynchObjects::init(EGetOpt &options)
{
   options.setPrefix(SECTION_TOOLS);
   if (options.get(MEMBER_DEBUG, false))
      options.print();
   bool enabled = options.get(MEMBER_ENABLE_PUBLIC_OBJECTS, false);
   options.setPrefix("");
   if (!enabled)
      return;

   options.setPrefix(SECTION_TOOLS "/" SECTION_SYNCH_OBJS);
   Int nSemaphores = options.get(MEMBER_NUMBER_SEMAPHORES, 0);
   Int nMutexes = options.get(MEMBER_NUMBER_MUTEXES, 0);
   options.setPrefix("");

   options.setPrefix(SECTION_TOOLS);
   UInt nPublicQueues = options.getCount(SECTION_PUBLIC_QUEUE);
   options.setPrefix("");

   m_sharedmem.init("ESynchObjectPublicStorage", 'A',
                    sizeof(Bool) +             // initialization flag
                    sizeof(esynchcontrol_t) + // synch control block
                    //sizeof(esynchcontrol_t) + // semaphore control block
                    //sizeof(esynchcontrol_t) + // mutex control block
                    //sizeof(esynchcontrol_t) + // condition variable control block
                    sizeof(ESemaphoreDataPublic) * nSemaphores +
                    sizeof(EMutexDataPublic) * nMutexes +                    // storage for mutex objects
                    sizeof(epublicqueuedef_t) * (nPublicQueues + 1) // storage for public queue definitions
   );

   m_pCtrl = (esynchcontrol_t *)m_sharedmem.getDataPtr();

   m_pSemaphores = (ESemaphoreDataPublic *)((pChar)m_pCtrl + sizeof(*m_pCtrl));
   m_pMutexes = (EMutexDataPublic *)((pChar)m_pSemaphores + (sizeof(ESemaphoreDataPublic) * nSemaphores));
   m_pPubQueues = (epublicqueuedef_t *)((pChar)m_pMutexes + (sizeof(EMutexDataPublic) * nMutexes));

   if (!m_pCtrl->m_initialized)
   {
      m_pCtrl->m_sequence = 0;

      new(&m_pCtrl->m_semaphoreCtrl.m_mutex) EMutexPrivate();
      new(&m_pCtrl->m_mutexCtrl.m_mutex) EMutexPrivate();

      EMutexLock ls(m_pCtrl->m_semaphoreCtrl.m_mutex);
      EMutexLock lm(m_pCtrl->m_mutexCtrl.m_mutex);

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

      memset(m_pPubQueues, 0, sizeof(epublicqueuedef_t) * (nPublicQueues + 1));

      m_pCtrl->m_initialized = True;
   }

   m_pThis = this;

   ////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////

   EString s;
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

Void ESynchObjects::uninit()
{
   // there should be no need to "uninit" since
   // if all users are done, the shared memory block
   // will just be thrown away (and all of the "uninit'ing"
   // will occur in the shared memory
   //
}

Void ESynchObjects::logObjectUsage()
{
   // ELOGINFO(ELOG_SYNCHOBJECTS, "Maximum number of semaphores allocated was %ld", m_pCtrl->m_semaphoreCtrl.m_maxused);
   // ELOGINFO(ELOG_SYNCHOBJECTS, "Maximum number of mutexes allocated was %ld", m_pCtrl->m_mutexCtrl.m_maxused);
}

Int ESynchObjects::nextSemaphore()
{
   ESynchObjects *pso = getSynchObjCtrlPtr();
   esynchcontrol_t *pctrl = pso->m_pCtrl;
   
   EMutexLock l(pctrl->m_semaphoreCtrl.m_mutex);

   if (pctrl->m_semaphoreCtrl.m_head == -1)
      throw ESemaphoreError_UnableToAllocateSemaphore();

   Int ofs = pctrl->m_semaphoreCtrl.m_head;
   pctrl->m_semaphoreCtrl.m_head =
       pso->m_pSemaphores[pctrl->m_semaphoreCtrl.m_head].nextIndex();

   pctrl->m_semaphoreCtrl.m_currused++;
   if (pctrl->m_semaphoreCtrl.m_currused > pctrl->m_semaphoreCtrl.m_maxused)
      pctrl->m_semaphoreCtrl.m_maxused = pctrl->m_semaphoreCtrl.m_currused;

   return ofs + 1;
}

Int ESynchObjects::nextMutex()
{
   ESynchObjects *pso = getSynchObjCtrlPtr();
   esynchcontrol_t *pctrl = pso->m_pCtrl;
   
   EMutexLock l(pctrl->m_mutexCtrl.m_mutex);

   if (pctrl->m_mutexCtrl.m_head == -1)
      throw EMutexError_UnableToAllocateMutex();

   Int ofs = pctrl->m_mutexCtrl.m_head;
   pctrl->m_mutexCtrl.m_head =
       pso->m_pMutexes[pctrl->m_mutexCtrl.m_head].nextIndex();

   pctrl->m_mutexCtrl.m_currused++;
   if (pctrl->m_mutexCtrl.m_currused > pctrl->m_mutexCtrl.m_maxused)
      pctrl->m_mutexCtrl.m_maxused = pctrl->m_mutexCtrl.m_currused;

   return ofs + 1;
}

Void ESynchObjects::freeSemaphore(Int nSemId)
{
   ESynchObjects *pso = getSynchObjCtrlPtr();
   esynchcontrol_t *pctrl = pso->m_pCtrl;
   
   EMutexLock l(pctrl->m_semaphoreCtrl.m_mutex);

   nSemId--;

   pso->m_pSemaphores[nSemId].nextIndex() = pctrl->m_semaphoreCtrl.m_head;
   pctrl->m_semaphoreCtrl.m_head = nSemId;

   pctrl->m_semaphoreCtrl.m_currused--;
}

Void ESynchObjects::freeMutex(Int nMutexId)
{
   ESynchObjects *pso = getSynchObjCtrlPtr();
   esynchcontrol_t *pctrl = pso->m_pCtrl;
   
   EMutexLock l(pctrl->m_mutexCtrl.m_mutex);

   nMutexId--;

   pso->m_pMutexes[nMutexId].nextIndex() = pctrl->m_mutexCtrl.m_head;
   pctrl->m_mutexCtrl.m_head = nMutexId;

   pctrl->m_mutexCtrl.m_currused--;
}
/// @endcond

////////////////////////////////////////////////////////////////////////////////
// Common Mutex Classes
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

Void EMutexData::init(Bool isPublic)
{
   if (!m_initialized)
   {
#if defined(NATIVE_IPC)
      Int res;
      pthread_mutexattr_t attr;

      if ((res = pthread_mutexattr_init(&attr)) != 0)
         throw EMutexError_UnableToInitialize(res);
      if (isPublic && (res = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) != 0)
         throw EMutexError_UnableToInitialize(res);
      if ((res = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) != 0)
         throw EMutexError_UnableToInitialize(res);
      if ((res = pthread_mutex_init(&m_mutex, &attr)) != 0)
         throw EMutexError_UnableToInitialize(res);
#else
      m_lock = 0;
#endif
      m_initialized = True;
   }
}

Void EMutexData::destroy()
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

Bool EMutexData::enter(Bool wait)
{
#if defined(NATIVE_IPC)
   Int res = pthread_mutex_lock(&data.mutex());
   if (res != 0 && wait)
      throw EMutexError_UnableToLock(res);
   return res == 0;
#else
   while (True)
   {
      if (atomic_cas(mutex(), 0, 1) == 0)
         return True;
      if (wait)
         EThreadBasic::yield();
      else
         return False;
   }
#endif
}

Void EMutexData::leave()
{
#if defined(NATIVE_IPC)
   Int res = pthread_mutex_unlock(&mutex());
   if (res != 0)
      throw EMutexError_UnableToUnLock(res);
#else
   atomic_swap(mutex(), 0);
#endif
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
// Public Mutex Classes
////////////////////////////////////////////////////////////////////////////////

Void EMutexPublic::init()
{
   if (m_mutexid != 0)
      throw EMutexError_UnableToAllocateMutex();

   m_mutexid = ESynchObjects::getSynchObjCtrlPtr()->nextMutex();
   ((EMutexDataPublic)*this).init(True);
}

Void EMutexPublic::destroy()
{
   if (m_mutexid == 0)
      return;

   ((EMutexDataPublic)*this).destroy();
   ESynchObjects::getSynchObjCtrlPtr()->freeMutex(m_mutexid);
   m_mutexid = 0;
}

EMutexPublic::operator EMutexDataPublic &()
{
   if (m_mutexid == 0)
      throw EMutexError_MutexUninitialized();
   return ESynchObjects::getSynchObjCtrlPtr()->getMutex(m_mutexid);
}

Void EMutexPublic::attach(Int mutexid)
{
   if (m_mutexid)
      throw EMutexError_AlreadyInitialized();
   m_mutexid = mutexid;
}

Void EMutexPublic::detach()
{
   if (m_mutexid == 0)
      throw EMutexError_MutexUninitialized();
   m_mutexid = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Common Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

Void ESemaphoreData::init()
{
   Int res;

   res = sem_init(&m_sem, shared() ? 1 : 0, 0);
   if (res == -1)
      throw ESemaphoreError_UnableToInitialize();
   
   m_currCount = m_initCount;
   m_initialized = True;
}

Void ESemaphoreData::destroy()
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

Bool ESemaphoreData::Decrement(Bool wait)
{
   if (!initialized())
      throw ESemaphoreError_NotInitialized();

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

Bool ESemaphoreData::Increment()
{
   if (!initialized())
      throw ESemaphoreError_NotInitialized();

   Long val = atomic_inc(m_currCount);
   if (val < 1)
   {
      if (sem_post(&m_sem) != 0)
      {
         atomic_dec(m_currCount);
         throw ESemaphoreError_UnableToIncrement();
      }
   }
   return True;
}

////////////////////////////////////////////////////////////////////////////////
// Public Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE
ESemaphoreData &ESemaphorePublic::getData()
{
   return ESynchObjects::getSemaphore(m_semid);
}
/// @endcond

Void ESemaphorePublic::init(Long initialCount)
{
   if (m_semid != 0)
      throw ESemaphoreError_AlreadyInitialized();

   m_semid = ESynchObjects::getSynchObjCtrlPtr()->nextSemaphore();
   ESemaphoreDataPublic &d = ESynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid);
   d.shared() = True;
   d.initialCount() = initialCount;
   d.init();
}

Void ESemaphorePublic::destroy()
{
   if (m_semid == 0)
      return;
   
   ESemaphoreDataPublic &d = ESynchObjects::getSemaphore(m_semid);

   d.destroy();
   ESynchObjects::freeSemaphore(m_semid);
   m_semid = 0;
}

/// @cond DOXYGEN_EXCLUDE
Int &ESemaphorePublic::nextIndex()
{
   if (m_semid == 0)
      throw ESemaphoreError_NotInitialized();
   return ESynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).nextIndex();      
}
/// @endcond

Int &ESemaphorePublic::semIndex()
{
   if (m_semid == 0)
      throw ESemaphoreError_NotInitialized();
   return ESynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).semIndex();      
}

Void ESemaphorePublic::attach(Int semid)
{
   if (m_semid)
      throw ESemaphoreError_AlreadyInitialized();
   m_semid = semid;
}

Void ESemaphorePublic::detach()
{
   if (m_semid == 0)
      throw ESemaphoreError_NotInitialized();
   m_semid = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Private Read/Write Lock Classes
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE
ERWLockError_LockAttrInitFailed::ERWLockError_LockAttrInitFailed(Int err)
{
   setSevere();
   setText("Unable to initialize read/write lock attribute ");
   appendLastOsError(err);
}

ERWLockError_LockInitFailed::ERWLockError_LockInitFailed(Int err)
{
   setSevere();
   setText("Unable to initialize read/write lock ");
   appendLastOsError(err);
}
/// @endcond

ERWLock::ERWLock()
{
   int status;
   pthread_rwlockattr_t attr;

   status = pthread_rwlockattr_init(&attr);
   if (status != 0)
      throw ERWLockError_LockAttrInitFailed(status);

   // set any applicable attributes

   status = pthread_rwlock_init( &m_rwlock, &attr );

   pthread_rwlockattr_destroy( &attr );

   if (status != 0)
      throw ERWLockError_LockInitFailed(status);
}

ERWLock::~ERWLock()
{
   pthread_rwlock_destroy( &m_rwlock );
}

/// @cond DOXYGEN_EXCLUDE
bool ERWLock::enter( ReadWrite rw, bool wait )
{
   int status;

   if ( wait )
   {
      if (rw == ERWLock::Read)
         status = pthread_rwlock_rdlock( &m_rwlock );
      else
         status = pthread_rwlock_wrlock( &m_rwlock );
   }
   else
   {
      if (rw == ERWLock::Read)
         status = pthread_rwlock_tryrdlock( &m_rwlock );
      else
         status = pthread_rwlock_trywrlock( &m_rwlock );
   }

   return status == 0;
}

void ERWLock::leave()
{
   pthread_rwlock_unlock( &m_rwlock );
}
/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

EEvent::EEvent( bool state )
   : m_buf()
{
   m_pipefd[0] = -1;
   m_pipefd[1] = -1;

   int result = pipe2( m_pipefd, O_NONBLOCK );
   if ( result == -1 )
      throw EError( EError::Warning, "Error createing pipe (EEvent)" );

   if ( state )
      set();
}

EEvent::~EEvent()
{
   closepipe();
}

void EEvent::set()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
   write( m_pipefd[1], &m_buf, 1 );
#pragma GCC diagnostic pop
}

void EEvent::reset()
{
   while ( read(m_pipefd[0], &m_buf, sizeof(m_buf)) > 0 );
}

bool EEvent::wait( int ms )
{
   bool rval = false;
   struct pollfd fds[] = { { .fd = m_pipefd[0], .events = POLLRDNORM } };

   while (true)
   {
      int result = poll( fds, 1, ms );
      if ( result > 0 ) // event set
      {
         rval = true;
      }
      else if ( result == 0 )
      {
         // timeout
      }
      else
      {
         if (errno == EINTR)
            continue;
      }
      break;
   }

   return rval;
}

void EEvent::closepipe()
{
   if ( m_pipefd[0] != -1 )
   {
      close( m_pipefd[0] );
      m_pipefd[0] = -1;
   }
   if ( m_pipefd[1] != -1 )
   {
      close( m_pipefd[1] );
      m_pipefd[1] = -1;
   }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
