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

#ifndef __ftsynch_h_included
#define __ftsynch_h_included

#include <semaphore.h>

#include "ftbase.h"
#include "ftstring.h"
#include "fterror.h"

#define MAX_SEMIDS 64
#define MAX_NOTIFYIDS 64

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED2(FTMutexError_UnableToInitialize);
DECLARE_ERROR_ADVANCED2(FTMutexError_UnableToLock);
DECLARE_ERROR_ADVANCED2(FTMutexError_UnableToUnLock);
DECLARE_ERROR(FTMutexError_UnableToAllocateMutex);
DECLARE_ERROR(FTMutexError_AlreadyInitialized);
DECLARE_ERROR(FTMutexError_MutexUninitialized);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED(FTSemaphoreError_UnableToInitialize);
DECLARE_ERROR_ADVANCED(FTSemaphoreError_UnableToDecrement);
DECLARE_ERROR_ADVANCED(FTSemaphoreError_UnableToIncrement);

DECLARE_ERROR(FTSemaphoreError_UnableToAllocateSemaphore);
DECLARE_ERROR(FTSemaphoreError_AlreadyAllocated);
DECLARE_ERROR(FTSemaphoreError_NotInitialized);
DECLARE_ERROR(FTSemaphoreError_AlreadyInitialized);
DECLARE_ERROR(FTSemaphoreError_MaxNotifyIdsExceeded);

////////////////////////////////////////////////////////////////////////////////
// Common Mutex Classes
////////////////////////////////////////////////////////////////////////////////

class FTMutexLock;

class FTMutexData
{
   friend FTMutexLock;

public:
   FTMutexData()
   {
      m_initialized = False;
   }
   ~FTMutexData()
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

#if 0
class FTMutexBase
{
   friend FTMutexLock;

public:
   FTMutexBase()
   {
   }
   ~FTMutexBase()
   {
   }

   virtual Void init() = 0;

   Void destroy()
   {
      FTMutexData &data = getData();
      data.destroy();
   }


protected:
   virtual FTMutexData &getData() = 0;

   Void init(Bool isPublic)
   {
      FTMutexData &data = getData();
      data.init(isPublic);
   }

   Bool enter(Bool wait = True);
   Void leave();
};
#endif

class FTMutexLock
{
public:
   FTMutexLock(FTMutexData &mtx, Bool acquire = True)
      : m_acquire(acquire), m_mtx(mtx)
   {
      if (m_acquire)
         m_mtx.enter();
   }

   ~FTMutexLock()
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
   FTMutexData &m_mtx;
   Bool m_acquire;

   FTMutexLock();
   FTMutexLock &operator=(const FTMutexLock &other);
};

////////////////////////////////////////////////////////////////////////////////
// Private Mutex Classes
////////////////////////////////////////////////////////////////////////////////

class FTMutexPrivate : public FTMutexData
{
public:
   FTMutexPrivate(Bool bInit = True)
       : FTMutexData()
   {
      if (bInit)
         init();
   }

   ~FTMutexPrivate() { destroy(); }

   Void *operator new(size_t, void *where) { return where; }

   Void init() { FTMutexData::init(False); }
};

////////////////////////////////////////////////////////////////////////////////
// Public Mutex Classes
////////////////////////////////////////////////////////////////////////////////

class FTMutexDataPublic : public FTMutexData
{
public:
   FTMutexDataPublic() {}
   ~FTMutexDataPublic() {}

   Int &nextIndex() { return m_nextIndex; }
   Int &mutexId() { return m_mutexId; }

private:
   Int m_nextIndex;
   Int m_mutexId;
};

class FTMutexPublic
{
public:
   FTMutexPublic(Bool bInit = True)
   {
      m_mutexid = 0;
      if (bInit)
         init();
   }

   ~FTMutexPublic()
   {
      destroy();
   }

   Void init();
   Void destroy();

   Void attach(Int mutexid);
   Void detach();

   Int mutexId() { return m_mutexid; }

   operator FTMutexDataPublic &();

private:
   Int m_mutexid;
};

////////////////////////////////////////////////////////////////////////////////
// Common Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

class FTSemaphoreData
{
public:
   FTSemaphoreData()
      : m_initialized(False),
        m_shared(False),
        m_initCount(0),
        m_currCount(0)
   {
   }

   FTSemaphoreData(Long initcnt, Bool shared)
      : m_initialized(False),
        m_shared(shared),
        m_initCount(initcnt),
        m_currCount(0)
   {
   }

   ~FTSemaphoreData()
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

class FTSemaphoreBase
{
public:
   FTSemaphoreBase() {}
   ~FTSemaphoreBase() {}

   virtual Void init(Long initcnt) = 0;
   virtual Void destroy() = 0;

   Bool Decrement(Bool wait = True) { return getData().Decrement(wait); }
   Bool Increment() { return getData().Increment(); }

   Bool initialized() { return getData().initialized(); }
   Bool &shared() { return getData().shared(); }
   Long &initialCount() { return getData().initialCount(); }
   Long currCount() { return getData().currCount(); }

   operator FTSemaphoreData&() { return getData(); }

protected:
   virtual FTSemaphoreData &getData() = 0;
};

////////////////////////////////////////////////////////////////////////////////
// Private Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

class FTSemaphorePrivate : public FTSemaphoreBase
{
public:
   FTSemaphorePrivate(Long initcnt=0, Bool bInit = True)
      : m_data(initcnt, False)
   {
      if (bInit)
         init(initcnt);
   }

   ~FTSemaphorePrivate()
   {
      destroy();
   }

   Void init(Long initcnt) { m_data.initialCount() = initcnt; m_data.init(); }
   Void destroy() { m_data.destroy(); }

protected:
   FTSemaphoreData &getData() { return m_data; }

private:
   FTSemaphoreData m_data;
};

////////////////////////////////////////////////////////////////////////////////
// Public Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

class FTSemaphoreDataPublic : public FTSemaphoreData
{
public:
   FTSemaphoreDataPublic(Long initcnt=0)
      : FTSemaphoreData(initcnt, True)
   {
   }

   ~FTSemaphoreDataPublic()
   {
   }

   Int &nextIndex() { return m_nextIndex; }
   Int &semIndex() { return m_semIndex; }

private:
   Int m_nextIndex;
   Int m_semIndex;
};

class FTSemaphorePublic : public FTSemaphoreBase
{
public:
   FTSemaphorePublic()
      : m_semid(0)
   {
   }
   FTSemaphorePublic(Long initcnt, Bool bInit = True)
      : m_semid(0)
   {
      if (bInit)
         init(initcnt);
   }

   ~FTSemaphorePublic()
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
   FTSemaphoreData &getData();

private:
   Int m_semid;
};

#if 0
class FTSemaphore;

class _FTSemaphore
{
   friend class FTSemaphore;
   friend class FTSynchObjects;

public:
   _FTSemaphore();
   _FTSemaphore(Long lInitialCount, Long lMaxCount, cpStr szName = NULL, Bool bInit = True);
   ~_FTSemaphore();

   Void init(Long lInitialCount, Long lMaxCount, cpStr szName = NULL);
   Void destroy(int semid);

   Long Decrement(Bool wait = True);
   Long Increment();

   Long getInitialCount() { return m_initialCount; }
   Long getMaxCount() { return m_maxCount; }
   Long getCurrCount() { return m_currCount; }
   cpStr getName() { return m_szName; }

   Int getSemId() { return m_semid; }
   Void setSemId(Int semid) { m_semid = semid; }

protected:
   Int getNextIndex() { return m_nextIndex; }
   Void setNextIndex(Int val) { m_nextIndex = val; }

private:
   Int m_nextIndex;

   Int m_semid;
   Bool m_initialized;
   Long m_currCount;

   Long m_initialCount;
   Long m_maxCount;
   Char m_szName[FT_FILENAME_MAX];

   sem_t m_sem;
};

class FTQueueBase;

class FTSemaphore
{
   friend class FTQueueBase;

public:
   FTSemaphore();
   FTSemaphore(Long lInitialCount, Long lMaxCount, cpStr szName = NULL, Bool bInit = True);
   ~FTSemaphore();

   Void init(Int semid);
   Void init(Long lInitialCount, Long lMaxCount, cpStr szName = NULL);
   Void destroy();

   Bool Decrement(Bool wait = True);
   Void Increment();

   Int &getSemid() { return m_semid; }

   Long getCurrCount();

protected:
   Void cleanup();

private:
   Int m_semid;
};
#endif

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#endif // #define __ftsynch_h_included
