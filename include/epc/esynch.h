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

#ifndef __esynch_h_included
#define __esynch_h_included

#include <semaphore.h>

#include "ebase.h"
#include "estring.h"
#include "eerror.h"

#define MAX_SEMIDS 64
#define MAX_NOTIFYIDS 64

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED2(EMutexError_UnableToInitialize);
DECLARE_ERROR_ADVANCED2(EMutexError_UnableToLock);
DECLARE_ERROR_ADVANCED2(EMutexError_UnableToUnLock);
DECLARE_ERROR(EMutexError_UnableToAllocateMutex);
DECLARE_ERROR(EMutexError_AlreadyInitialized);
DECLARE_ERROR(EMutexError_MutexUninitialized);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED(ESemaphoreError_UnableToInitialize);
DECLARE_ERROR_ADVANCED(ESemaphoreError_UnableToDecrement);
DECLARE_ERROR_ADVANCED(ESemaphoreError_UnableToIncrement);

DECLARE_ERROR(ESemaphoreError_UnableToAllocateSemaphore);
DECLARE_ERROR(ESemaphoreError_AlreadyAllocated);
DECLARE_ERROR(ESemaphoreError_NotInitialized);
DECLARE_ERROR(ESemaphoreError_AlreadyInitialized);
DECLARE_ERROR(ESemaphoreError_MaxNotifyIdsExceeded);

////////////////////////////////////////////////////////////////////////////////
// Common Mutex Classes
////////////////////////////////////////////////////////////////////////////////

class EMutexLock;

class EMutexData
{
   friend EMutexLock;

public:
   EMutexData()
   {
      m_initialized = False;
   }
   ~EMutexData()
   {
   }

   Void init(Bool shared);
   Void destroy();

   Bool initialized()
   {
      return m_initialized;
   }

#if defined(NATIVE_IPC)
   pthread_mutex_t &mutex()
   {
      return m_mutex;
   }
#else
   Long &mutex()
   {
      return m_lock;
   }
#endif

protected:
   Bool enter(Bool wait = True);
   Void leave();

private:
   Bool m_initialized;
#if defined(NATIVE_IPC)
   pthread_mutex_t m_mutex;
#else
   Long m_lock;
#endif
};

class EMutexLock
{
public:
   EMutexLock(EMutexData &mtx, Bool acquire = True)
      : m_acquire(acquire), m_mtx(mtx)
   {
      if (m_acquire)
         m_mtx.enter();
   }

   ~EMutexLock()
   {
      if (m_acquire)
         m_mtx.leave();
   }

   Bool acquire(Bool wait = True)
   {
      if (!m_acquire)
         m_acquire = m_mtx.enter(wait);
      return m_acquire;
   }

private:
   EMutexData &m_mtx;
   Bool m_acquire;

   EMutexLock();
   EMutexLock &operator=(const EMutexLock &other);
};

////////////////////////////////////////////////////////////////////////////////
// Private Mutex Classes
////////////////////////////////////////////////////////////////////////////////

class EMutexPrivate : public EMutexData
{
public:
   EMutexPrivate(Bool bInit = True)
       : EMutexData()
   {
      if (bInit)
         init();
   }

   ~EMutexPrivate() { destroy(); }

   Void *operator new(size_t, void *where) { return where; }

   Void init() { EMutexData::init(False); }
};

////////////////////////////////////////////////////////////////////////////////
// Public Mutex Classes
////////////////////////////////////////////////////////////////////////////////

class EMutexDataPublic : public EMutexData
{
public:
   EMutexDataPublic() {}
   ~EMutexDataPublic() {}

   Int &nextIndex() { return m_nextIndex; }
   Int &mutexId() { return m_mutexId; }

private:
   Int m_nextIndex;
   Int m_mutexId;
};

class EMutexPublic
{
public:
   EMutexPublic(Bool bInit = True)
   {
      m_mutexid = 0;
      if (bInit)
         init();
   }

   ~EMutexPublic()
   {
      destroy();
   }

   Void init();
   Void destroy();

   Void attach(Int mutexid);
   Void detach();

   Int mutexId() { return m_mutexid; }

   operator EMutexDataPublic &();

private:
   Int m_mutexid;
};

////////////////////////////////////////////////////////////////////////////////
// Common Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

class ESemaphoreData
{
public:
   ESemaphoreData()
      : m_initialized(False),
        m_shared(False),
        m_initCount(0),
        m_currCount(0)
   {
   }

   ESemaphoreData(Long initcnt, Bool shared)
      : m_initialized(False),
        m_shared(shared),
        m_initCount(initcnt),
        m_currCount(0)
   {
   }

   ~ESemaphoreData()
   {
      destroy();
   }

   Void init();
   Void destroy();

   Bool Decrement(Bool wait = True);
   Bool Increment();

   Bool initialized() { return m_initialized; }
   Bool &shared() { return m_shared; }
   Long &initialCount() { return m_initCount; }
   Long currCount() { return m_currCount; }

private:
   Bool m_initialized;
   Bool m_shared;
   Long m_initCount;
   Long m_currCount;
   sem_t m_sem;
};

class ESemaphoreBase
{
public:
   ESemaphoreBase() {}
   ~ESemaphoreBase() {}

   virtual Void init(Long initcnt) = 0;
   virtual Void destroy() = 0;

   Bool Decrement(Bool wait = True) { return getData().Decrement(wait); }
   Bool Increment() { return getData().Increment(); }

   Bool initialized() { return getData().initialized(); }
   Bool &shared() { return getData().shared(); }
   Long &initialCount() { return getData().initialCount(); }
   Long currCount() { return getData().currCount(); }

   operator ESemaphoreData&() { return getData(); }

protected:
   virtual ESemaphoreData &getData() = 0;
};

////////////////////////////////////////////////////////////////////////////////
// Private Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

class ESemaphorePrivate : public ESemaphoreBase
{
public:
   ESemaphorePrivate(Long initcnt=0, Bool bInit = True)
      : m_data(initcnt, False)
   {
      if (bInit)
         init(initcnt);
   }

   ~ESemaphorePrivate()
   {
      destroy();
   }

   Void init(Long initcnt) { m_data.initialCount() = initcnt; m_data.init(); }
   Void destroy() { m_data.destroy(); }

protected:
   ESemaphoreData &getData() { return m_data; }

private:
   ESemaphoreData m_data;
};

////////////////////////////////////////////////////////////////////////////////
// Public Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

class ESemaphoreDataPublic : public ESemaphoreData
{
public:
   ESemaphoreDataPublic(Long initcnt=0)
      : ESemaphoreData(initcnt, True)
   {
   }

   ~ESemaphoreDataPublic()
   {
   }

   Int &nextIndex() { return m_nextIndex; }
   Int &semIndex() { return m_semIndex; }

private:
   Int m_nextIndex;
   Int m_semIndex;
};

class ESemaphorePublic : public ESemaphoreBase
{
public:
   ESemaphorePublic()
      : m_semid(0)
   {
   }
   ESemaphorePublic(Long initcnt, Bool bInit = True)
      : m_semid(0)
   {
      if (bInit)
         init(initcnt);
   }

   ~ESemaphorePublic()
   {
      destroy();
   }

   Void init(Long initialCount);
   Void destroy();

   Int &nextIndex();
   Int &semIndex();

   Void attach(Int semid);
   Void detach();

protected:
   ESemaphoreData &getData();

private:
   Int m_semid;
};

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#endif // #define __esynch_h_included
