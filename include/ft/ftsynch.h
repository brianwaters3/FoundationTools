////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __ftsynch_h_included
#define __ftsynch_h_included

#include "ftbase.h"
#include "ftstring.h"
#include "fterror.h"

#if defined(FT_WINDOWS)
#include "ftautohandle.h"
#elif defined(FT_GCC)
#include <semaphore.h>
#elif defined(FT_SOLARIS)
#include <semaphore.h>
#else
#error "Unrecoginzed platform"
#endif

#define MAX_SEMIDS      64
#define MAX_NOTIFYIDS   64

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED2(FTMutexError_UnableToInitialize);
DECLARE_ERROR_ADVANCED2(FTMutexError_UnableToLock);
DECLARE_ERROR_ADVANCED2(FTMutexError_UnableToUnLock);
DECLARE_ERROR(FTMutexError_UnableToAllocateMutex);

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
////////////////////////////////////////////////////////////////////////////////

class FTSynchObjects;
class FTMutex;
class FTSharedMemory;
class _FTMutexLock;
class _FTSemaphore;
class FTThreadBasic;
class FTQueueBase;

class _FTMutex
{
public:
    _FTMutex(Bool bInit = True);
    ~_FTMutex();

    Void init(cpStr pName);
    Void destroy();

    Bool enter(Bool wait = True);
    Void leave();

    Int getNextIndex() { return m_nextIndex; }
    Void setNextIndex(Int val) { m_nextIndex = val; }

    Int getMutexId() { return m_mutexid; }
    Void setMutexId(Int mutexid) { m_mutexid = mutexid; }

private:
    Int m_nextIndex;

    Bool m_initialized;
    Int m_mutexid;
#if defined(FT_WINDOWS)
    Long m_lock;
#elif defined(FT_GCC) && defined(NATIVE_IPC)
    pthread_mutex_t m_mutex;
#elif defined(FT_GCC)
    Long m_lock;
#elif defined(FT_SOLARIS) && defined(NATIVE_IPC)
    pthread_mutex_t m_mutex;
#elif defined(FT_SOLARIS)
    Long m_lock;
#else
#error "Unrecognized platform"
#endif
};

class FTMutex
{
public:
    FTMutex(Bool bInit = True);
    ~FTMutex();

    Void init(cpStr pName);
    Void destroy();

    Bool enter(Bool wait = True);
    Void leave();

protected:
    _FTMutex& getHandle();

private:
    Int m_mutexid;
};

class _FTMutexLock
{
public:
    _FTMutexLock(_FTMutex &mtx, Bool acquire = true)
        : m_acquire(acquire), m_mtx(mtx)
    {
        if (m_acquire)
            m_mtx.enter();
    }

    ~_FTMutexLock()
    {
        if (m_acquire)
            m_mtx.leave();
    }

private:
    Bool m_acquire;
    _FTMutex &m_mtx;

    // make these private to disallow them:
    _FTMutexLock &operator = (const _FTMutexLock &other)
    {
        return  *this;
    }
};

class FTMutexLock
{
public:
    FTMutexLock(FTMutex &mtx, Bool acquire = True)
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
    Bool m_acquire;
    FTMutex &m_mtx;

    // make these private to disallow them:
    FTMutexLock &operator = (const FTMutexLock &other)
    {
        return  *this;
    }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

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

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
    sem_t m_sem;
#elif defined(FT_SOLARIS)
    sem_t m_sem;
#else
#error "Unrecoginzed platform"
#endif
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

    Int& getSemid() { return m_semid; }

	Long getCurrCount();

protected:
    Void cleanup();

private:
    Int m_semid;

#if defined(FT_WINDOWS)
    FTAutoHandle m_handle;
#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
};

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#endif // #define __ftsynch_h_included

