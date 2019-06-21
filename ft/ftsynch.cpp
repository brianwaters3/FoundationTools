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

#include "ftinternal.h"
#include "ftsynch.h"
#include "ftsynch2.h"
#include "ftthread.h"

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
#include <errno.h>
#elif defined(FT_SOLARIS)
#include <errno.h>
#else
#error "Unrecognized platform"
#endif

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
FTSynchObjects* FTSynchObjects::m_pThis = NULL;

Void FTSynchObjects::FTSynchObjectsSharedMemory::onDestroy()
{
}

Void FTSynchObjects::FTSynchObjectsSharedMemory::setSynchObjectsPtr(FTSynchObjects* p)
{
    //FTLOGFUNC("FTSynchObjects::FTSynchObjectsSharedMemory::setSynchObjectsPtr");

    m_pSynchObjects = p;
}

FTSynchObjects::FTSynchObjects()
{
    //FTLOGFUNC("FTSynchObjects::FTSynchObjects");

    m_pCtrl = NULL;
    m_pSemaphores = NULL;
    m_pMutexes = NULL;

    m_pThis = this;
}

FTSynchObjects::~FTSynchObjects()
{
    //FTLOGFUNC("FTSynchObjects::~FTSynchObjects");
}

Void FTSynchObjects::init(FTGetOpt& options)
{
    //FTLOGFUNC("FTSynchObjects::init");

    options.setPrefix(SECTION_TOOLS);
    if (options.get(MEMBER_DEBUG,false))
        options.print();
    options.setPrefix("");

    options.setPrefix(SECTION_TOOLS "/" SECTION_SYNCH_OBJS);
    Int nSemaphores = options.get(MEMBER_NUMBER_SEMAPHORES, 0);
    Int nMutexes = options.get(MEMBER_NUMBER_MUTEXES, 0);
    options.setPrefix("");

    options.setPrefix(SECTION_TOOLS);
    UInt nPublicQueues = options.getCount(SECTION_PUBLIC_QUEUE);
    options.setPrefix("");

    m_sharedmem.init("FTSynchObjectPublicStorage", 'A',
            sizeof(Bool) + // initialization flag
            sizeof(ftsynchcontrol_t) + // semaphore control block
            sizeof(ftsynchcontrol_t) + // mutex control block
            sizeof(ftsynchcontrol_t) + // condition variable control block
            sizeof(_FTSemaphore) * nSemaphores +
            sizeof(_FTMutex) * nMutexes + // storage for mutex objects
            sizeof(ftpublicqueuedef_t) * (nPublicQueues + 1) // storage for public queue definitions
            );

    m_pCtrl = (ftsynchcontrol_t*)m_sharedmem.getDataPtr();

    m_pSemaphores = (_FTSemaphore*)((pChar)m_pCtrl + sizeof(*m_pCtrl));
    m_pMutexes = (_FTMutex*)((pChar)m_pSemaphores + (sizeof(_FTSemaphore) * nSemaphores));
    m_pPubQueues = (ftpublicqueuedef_t*)((pChar)m_pMutexes + (sizeof(_FTMutex) * nMutexes));

    if (!m_pCtrl->m_initialized)
    {
        m_pCtrl->m_sequence = 0;

        m_pCtrl->m_semaphoreCtrl.m_mutex.init(NULL);
        m_pCtrl->m_mutexCtrl.m_mutex.init(NULL);

        _FTMutexLock ls(m_pCtrl->m_semaphoreCtrl.m_mutex);
        _FTMutexLock lm(m_pCtrl->m_mutexCtrl.m_mutex);

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
            m_pSemaphores[ofs - 1].setSemId(ofs);
            m_pSemaphores[ofs - 1].setNextIndex(ofs);
        }
        m_pSemaphores[ofs - 1].setSemId(ofs);
        m_pSemaphores[ofs - 1].setNextIndex(-1);

        for (ofs = 1; ofs < m_pCtrl->m_mutexCtrl.m_max; ofs++)
        {
            m_pMutexes[ofs - 1].setMutexId(ofs);
            m_pMutexes[ofs - 1].setNextIndex(ofs);
        }
        m_pMutexes[ofs - 1].setMutexId(ofs);
        m_pMutexes[ofs - 1].setNextIndex(-1);

        memset(m_pPubQueues, 0, sizeof(ftpublicqueuedef_t) * (nPublicQueues + 1));

        m_pCtrl->m_initialized = True;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    FTString s;
    options.setPrefix(SECTION_TOOLS);
    for (UInt idx=0; idx<nPublicQueues; idx++)
    {
        Int queueid = options.get(idx, SECTION_PUBLIC_QUEUE, MEMBER_QUEUE_ID, -1);
        Int msgsize = options.get(idx, SECTION_PUBLIC_QUEUE, MEMBER_MESSAGE_SIZE, -1);
        Int queuesize = options.get(idx, SECTION_PUBLIC_QUEUE, MEMBER_QUEUE_SIZE, -1);
        Bool bR = options.get(idx, SECTION_PUBLIC_QUEUE, MEMBER_ALLOW_MULTIPLE_READERS, false);
        Bool bW = options.get(idx, SECTION_PUBLIC_QUEUE, MEMBER_ALLOW_MULTIPLE_WRITERS, false);

        s.format(SECTION_PUBLIC_QUEUE"%u", idx);
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
    //FTLOGFUNC("FTSynchObjects::uninit");

    // there should be no need to "uninit" since
    // if all users are done, the shared memory block
    // will just be thrown away (and all of the "uninit'ing"
    // will occur in the shared memory
    //
}

Void FTSynchObjects::logObjectUsage()
{
    FTLOGFUNC("FTSynchObjects::logObjectUsage");

    FTLOGINFO(FTLOG_SYNCHOBJECTS, "Maximum number of semaphores allocated was %ld", m_pCtrl->m_semaphoreCtrl.m_maxused);
    FTLOGINFO(FTLOG_SYNCHOBJECTS, "Maximum number of mutexes allocated was %ld", m_pCtrl->m_mutexCtrl.m_maxused);
}

Int FTSynchObjects::nextSemaphore()
{
    //FTLOGFUNC("FTSynchObjects::nextSemaphore");

    _FTMutexLock l(m_pCtrl->m_semaphoreCtrl.m_mutex);

    if (m_pCtrl->m_semaphoreCtrl.m_head == -1)
        throw new FTSemaphoreError_UnableToAllocateSemaphore();

    Int ofs = m_pCtrl->m_semaphoreCtrl.m_head;
    m_pCtrl->m_semaphoreCtrl.m_head =
        m_pSemaphores[m_pCtrl->m_semaphoreCtrl.m_head].getNextIndex();

    m_pCtrl->m_semaphoreCtrl.m_currused++;
    if (m_pCtrl->m_semaphoreCtrl.m_currused > m_pCtrl->m_semaphoreCtrl.m_maxused)
        m_pCtrl->m_semaphoreCtrl.m_maxused = m_pCtrl->m_semaphoreCtrl.m_currused;

    return ofs + 1;
}

Int FTSynchObjects::nextMutex()
{
    //FTLOGFUNC("FTSynchObjects::nextMutex");

    _FTMutexLock l(m_pCtrl->m_mutexCtrl.m_mutex);

    if (m_pCtrl->m_mutexCtrl.m_head == -1)
        throw new FTMutexError_UnableToAllocateMutex();

    Int ofs = m_pCtrl->m_mutexCtrl.m_head;
    m_pCtrl->m_mutexCtrl.m_head =
        m_pMutexes[m_pCtrl->m_mutexCtrl.m_head].getNextIndex();

    m_pCtrl->m_mutexCtrl.m_currused++;
    if (m_pCtrl->m_mutexCtrl.m_currused > m_pCtrl->m_mutexCtrl.m_maxused)
        m_pCtrl->m_mutexCtrl.m_maxused = m_pCtrl->m_mutexCtrl.m_currused;

    return ofs + 1;
}

Void FTSynchObjects::freeSemaphore(Int nSemId)
{
    //FTLOGFUNC("FTSynchObjects::freeSemaphore");

    _FTMutexLock l(m_pCtrl->m_semaphoreCtrl.m_mutex);

    nSemId--;

    m_pSemaphores[nSemId].setNextIndex(m_pCtrl->m_semaphoreCtrl.m_head);
    m_pCtrl->m_semaphoreCtrl.m_head = nSemId;

    m_pCtrl->m_semaphoreCtrl.m_currused--;
}

Void FTSynchObjects::freeMutex(Int nMutexId)
{
    //FTLOGFUNC("FTSynchObjects::freeMutex");

    _FTMutexLock l(m_pCtrl->m_mutexCtrl.m_mutex);

    nMutexId--;

    m_pMutexes[nMutexId].setNextIndex(m_pCtrl->m_mutexCtrl.m_head);
    m_pCtrl->m_mutexCtrl.m_head = nMutexId;

    m_pCtrl->m_mutexCtrl.m_currused--;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

_FTMutex::_FTMutex(Bool bInit)
    : m_initialized(False)
#if defined(FT_WINDOWS)
      ,m_lock(0)
#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
#else
#error "Unrecognized platform"
#endif
{
    //FTLOGFUNC("_FTMutex::_FTMutex");

    if (bInit)
        init(NULL);
}

_FTMutex::~_FTMutex()
{
    //FTLOGFUNC("_FTMutex::~_FTMutex");

    destroy();
}

Void _FTMutex::init(cpStr pName)
{
    //FTLOGFUNC("_FTMutex::init");

    if (!m_initialized)
    {
#if defined(FT_WINDOWS)
        m_lock = 0;
#elif defined(FT_SOLARIS) && defined(NATIVE_IPC)
        Int res;
        pthread_mutexattr_t attr;

        if ((res = pthread_mutexattr_init(&attr)) != 0)
            throw new FTMutexError_UnableToInitialize(res);
        else if ((res = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) != 0)
            throw new FTMutexError_UnableToInitialize(res);
        else if ((res = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) != 0)
            throw new FTMutexError_UnableToInitialize(res);
        else if ((res = pthread_mutex_init(&m_mutex, &attr)) != 0)
            throw new FTMutexError_UnableToInitialize(res);
#elif defined(FT_GCC)
        m_lock = 0;
#elif defined(FT_SOLARIS)
        m_lock = 0;
#else
#error "Unrecognized platform"
#endif
        m_initialized = True;
    }
}

Void _FTMutex::destroy()
{
    //FTLOGFUNC("_FTMutex::destroy");

    if (m_initialized)
    {
#if defined(FT_WINDOWS)
        m_lock = 0;
#elif defined(FT_GCC) && defined(NATIVE_IPC)
        pthread_mutex_destroy(&m_mutex);
#elif defined(FT_GCC)
        m_lock = 0;
#elif defined(FT_SOLARIS) && defined(NATIVE_IPC)
        pthread_mutex_destroy(&m_mutex);
#elif defined(FT_SOLARIS)
        m_lock = 0;
#else
#error "Unrecognized platform"
#endif
        m_initialized = False;
    }
}

Bool _FTMutex::enter(Bool wait)
{
    //FTLOGFUNC("_FTMutex::enter");

#if defined(FT_WINDOWS)
    Int cnt = 0;

    while (True)
    {
        if (!cnt && atomic_cas(m_lock,0,1) == 0)
            return True;
        cnt++;
        if (wait)
        {
            if (cnt >= 50)
            {
                FTThreadBasic::yield();
                cnt = 0;
            }
        }
        else
        {
            break;
        }
    }
    return False;

//    do
//    {
//        cnt++;
//        if (cnt >= 50)
//        {
//            FTThreadBasic::yield();
//            cnt = 0;
//        }
//    } while (atomic_cas(m_lock,0,1) == 1);
#elif defined(FT_SOLARIS) && defined(NATIVE_IPC)
    Int res = pthread_mutex_lock(&m_mutex);
    if (res != 0 && wait)
        throw new FTMutexError_UnableToLock(res);
    return res == 0;
#elif defined(FT_GCC)
    while (True)
    {
        if (atomic_cas(m_lock,0,1) == 0)
            return True;
        if (wait)
            FTThreadBasic::yield();
        else
            break;
    }
    return False;
//    while (atomic_cas(m_lock,0,1) == 1)
//        FTThreadBasic::yield();
#elif defined(FT_SOLARIS)
    do
    {
    while (True)
    {
        if (atomic_cas(m_lock,0,1) == 0)
            return True;
        if (wait)
            FTThreadBasic::yield();
        else
            break;
    }
    return False;
//    while (atomic_cas(m_lock,0,1) == 1)
//        FTThreadBasic::yield();
#else
#error "Unrecognized platform"
#endif
}

Void _FTMutex::leave()
{
    //FTLOGFUNC("_FTMutex::leave");

#if defined(FT_WINDOWS)
    atomic_swap(m_lock,0);
#elif defined(FT_GCC) && defined(NATIVE_IPC)
    Int res = pthread_mutex_unlock(&m_mutex);
    if (res != 0)
        throw new FTMutexError_UnableToUnLock(res);
#elif defined(FT_GCC)
    atomic_swap(m_lock,0);
#elif defined(FT_SOLARIS) && defined(NATIVE_IPC)
    Int res = pthread_mutex_unlock(&m_mutex);
    if (res != 0)
        throw new FTMutexError_UnableToUnLock(res);
#elif defined(FT_SOLARIS)
    atomic_swap(m_lock,0);
#else
#error "Unrecognized platform"
#endif
}

FTMutex::FTMutex(Bool bInit)
{
    //FTLOGFUNC("FTMutex::FTMutex");

    m_mutexid = 0;
    if (bInit)
        init(NULL);
}

FTMutex::~FTMutex()
{
    //FTLOGFUNC("FTMutex::~FTMutex");

    destroy();
}

Void FTMutex::init(cpStr pName)
{
    //FTLOGFUNC("FTMutex::init");

    if (m_mutexid != 0)
        throw new FTMutexError_UnableToAllocateMutex();

    m_mutexid = FTSynchObjects::getSynchObjCtrlPtr()->nextMutex();

    FTSynchObjects::getSynchObjCtrlPtr()->getMutex(m_mutexid).init(pName);
}

Void FTMutex::destroy()
{
    //FTLOGFUNC("FTMutex::destroy");

    if (m_mutexid == 0)
        return;

    FTSynchObjects::getSynchObjCtrlPtr()->getMutex(m_mutexid).destroy();
    FTSynchObjects::getSynchObjCtrlPtr()->freeMutex(m_mutexid);
    m_mutexid = 0;
}

Bool FTMutex::enter(Bool wait)
{
    //FTLOGFUNC("FTMutex::enter");

    return FTSynchObjects::getSynchObjCtrlPtr()->getMutex(m_mutexid).enter(wait);
}

Void FTMutex::leave()
{
    //FTLOGFUNC("FTMutex::leave");

    FTSynchObjects::getSynchObjCtrlPtr()->getMutex(m_mutexid).leave();
}

_FTMutex& FTMutex::getHandle()
{
    //FTLOGFUNC("FTMutex::getHandle");

    return FTSynchObjects::getSynchObjCtrlPtr()->getMutex(m_mutexid);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

_FTSemaphore::_FTSemaphore()
{
    //FTLOGFUNC("_FTSemaphore::_FTSemaphore");

    m_szName[0] = 0;
}

_FTSemaphore::_FTSemaphore(Long lInitialCount, Long lMaxCount, cpStr szName, Bool bInit)
{
    //FTLOGFUNC("_FTSemaphore::_FTSemaphore");

    m_szName[0] = 0;

    if (bInit)
        init(lInitialCount, lMaxCount, szName);
}

_FTSemaphore::~_FTSemaphore()
{
    //FTLOGFUNC("_FTSemaphore::~_FTSemaphore");

    destroy(0);
}

Void _FTSemaphore::init(Long lInitialCount, Long lMaxCount, cpStr szName)
{
    //FTLOGFUNC("_FTSemaphore::init");

    if (szName != NULL)
        ft_strcpy_s(m_szName, sizeof(m_szName), szName);

    m_initialCount = lInitialCount;
    m_currCount = lInitialCount;
    m_maxCount = lMaxCount;

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
    Int res;

    //res = sem_init(&m_sem, (szName != NULL) ? 1 : 0, lMaxCount);
    //res = sem_init(&m_sem, (szName != NULL) ? 1 : 0, lInitialCount);
    res = sem_init(&m_sem, (szName != NULL) ? 1 : 0, 0);
    if (res ==  -1)
        throw new FTSemaphoreError_UnableToInitialize();

    //for (Long i = lMaxCount; i > lInitialCount; i--)
    //    sem_wait(&m_sem);
#elif defined(FT_SOLARIS)
    Int res;

    res = sem_init(&m_sem, (szName != NULL) ? 1 : 0, lMaxCount);
    if (res ==  -1)
        throw new FTSemaphoreError_UnableToInitialize();

    for (Long i = lMaxCount; i > lInitialCount; i--)
        sem_wait(&m_sem);
#else
#error "Unrecoginzed platform"
#endif

    m_initialized = True;
}

Void _FTSemaphore::destroy(Int semid)
{
    FTLOGFUNC("_FTSemaphore::destroy");

    if (m_initialized)
    {
        m_initialCount = 0;
        m_currCount = 0;
        m_maxCount = 0;

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
        sem_destroy(&m_sem);
#elif defined(FT_SOLARIS)
        sem_destroy(&m_sem);
#else
#error "Unrecoginzed platform"
#endif

        m_initialized = False;
    }
    else
    {
        FTLOGERROR(FTLOG_SEMAPHORE, "semaphore %d is not initialized", getSemId());
    }
}

Long _FTSemaphore::Decrement(Bool wait)
{
    //FTLOGFUNC("_FTSemaphore::Decrement");

#if defined(FT_WINDOWS)
    return atomic_dec(m_currCount);
#elif defined(FT_GCC)
    Long val = atomic_dec(m_currCount);
    if (val < 0)
    {
        if (wait)
            val = sem_wait(&m_sem);
        else
            atomic_inc(m_currCount);
    }
    return val;
#elif defined(FT_SOLARIS)
    Long val = atomic_dec(m_currCount);
    if (val < 0)
    {
        if (wait)
            val = sem_wait(&m_sem);
        else
            atomic_inc(m_currCount);
    }
    return val;
#else
#error "Unrecoginzed platform"
#endif
}

Long _FTSemaphore::Increment()
{
    //FTLOGFUNC("_FTSemaphore::Increment");

#if defined(FT_WINDOWS)
    return atomic_inc(m_currCount);
#elif defined(FT_GCC)
    Long val = atomic_inc(m_currCount);
    if (val < 1)
    {
        if (sem_post(&m_sem) != 0)
            throw new FTSemaphoreError_UnableToIncrement();
    }
    return val;
#elif defined(FT_SOLARIS)
    Long val = atomic_inc(m_currCount);
    if (val < 1)
    {
        if (sem_post(&m_sem) != 0)
            throw new FTSemaphoreError_UnableToIncrement();
    }
    return val;
#else
#error "Unrecoginzed platform"
#endif
}

////////////////////////////////////////////////////////////////////////////////

FTSemaphore::FTSemaphore()
{
    //FTLOGFUNC("FTSemaphore::FTSemaphore");

    m_semid = 0;
}

FTSemaphore::FTSemaphore(Long lInitialCount, Long lMaxCount, cpStr szName, Bool bInit)
{
    //FTLOGFUNC("FTSemaphore::FTSemaphore");

    m_semid = 0;

    if (bInit)
        init(lInitialCount, lMaxCount, szName);
}

FTSemaphore::~FTSemaphore()
{
    //FTLOGFUNC("FTSemaphore::~FTSemaphore");

    destroy();
}

Void FTSemaphore::init(Int semid)
{
    FTLOGFUNC("FTSemaphore::init");

    if (m_semid != 0)
    {
        FTLOGERROR(FTLOG_SEMAPHORE, "semahore %d already initialized",
            FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).getName());
        throw new FTSemaphoreError_UnableToInitialize();
    }

    m_semid = semid;

#if defined(FT_WINDOWS)
    m_handle = CreateSemaphore(NULL, 0, 500,
        FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).getName());
    if (!m_handle.isvalid())
        throw new FTSemaphoreError_UnableToInitialize();
#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
}

Void FTSemaphore::init(Long lInitialCount, Long lMaxCount, cpStr szName)
{
    FTLOGFUNC("FTSemaphore::init");

    if (m_semid != 0)
    {
        FTLOGERROR(FTLOG_SEMAPHORE, "semahore %d already initialized", FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).getName());
        throw new FTSemaphoreError_UnableToInitialize();
    }

    m_semid = FTSynchObjects::getSynchObjCtrlPtr()->nextSemaphore();
    FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).init(lInitialCount, lMaxCount, szName);

#if defined(FT_WINDOWS)
    m_handle = CreateSemaphore(NULL, 0, 500, szName);
#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
}

Void FTSemaphore::destroy()
{
    //FTLOGFUNC("FTSemaphore::destroy");

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
    if (m_semid == 0)
        return;

    cleanup();

    FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).destroy(m_semid);
    FTSynchObjects::getSynchObjCtrlPtr()->freeSemaphore(m_semid);
    m_semid = 0;
#elif defined(FT_SOLARIS)
    if (m_semid == 0)
        return;

    cleanup();

    FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).destroy(m_semid);
    FTSynchObjects::getSynchObjCtrlPtr()->freeSemaphore(m_semid);
    m_semid = 0;
#else
#error "Unrecoginzed platform"
#endif
}

Void FTSemaphore::cleanup()
{
#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
}

Long FTSemaphore::getCurrCount()
{
	return FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).getCurrCount();
}

Bool FTSemaphore::Decrement(Bool wait)
{
    //FTLOGFUNC("FTSemaphore::Decrement");

    Long val = FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).Decrement(wait);

#if defined(FT_WINDOWS)
	Bool res = True;
    if (val < 0)
    {
        if (wait)
        {
            switch (WaitForSingleObjectEx(m_handle, wait ? INFINITE : 0, TRUE))
            {
                case WAIT_IO_COMPLETION:
                case WAIT_TIMEOUT:
                {
                    res = False;
                    break;
                }
                case WAIT_OBJECT_0:
                case WAIT_ABANDONED:
                {
                    res = True;
                    break;
                }
                case WAIT_FAILED:
                {
                    throw new FTSemaphoreError_UnableToDecrement();
                }
            }
        }
        else
        {
            FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).Increment();
            res = False;
        }
    }

    return res;
#elif defined(FT_GCC)
    return val < 0 ? False : True;
#elif defined(FT_SOLARIS)
    return val < 0 ? False : True;
#else
#error "Unrecoginzed platform"
#endif
}

#if defined (FT_WINDOWS)
#elif defined(FT_GCC)
#pragma GCC diagnostic ignored "-Wunused-variable"
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
Void FTSemaphore::Increment()
{
    FTLOGFUNC("FTSemaphore::Increment");

    if (FTSynchObjects::getSynchObjCtrlPtr()->getSemaphore(m_semid).Increment() < 1)
    {
        #if defined(FT_WINDOWS)
        if (!ReleaseSemaphore(m_handle, 1, NULL))
        {
            FTLOGERROR(FTLOG_SEMNOTICE, "ReleaseSemaphore returned FALSE (GetLastError() - %ld)", GetLastError());
        }
        #elif defined(FT_GCC)
        #elif defined(FT_SOLARIS)
        #else
        #error "Unrecoginzed platform"
        #endif
    }
}
#if defined (FT_WINDOWS)
#elif defined(FT_GCC)
#pragma GCC diagnostic warning "-Wunused-variable"
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
