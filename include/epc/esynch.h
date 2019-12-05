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

#ifndef __esynch_h_included
#define __esynch_h_included

/// @file
/// @brief Contains definitions for synchronization objects.

#include <semaphore.h>

#include "ebase.h"
#include "estring.h"
#include "eerror.h"

#define MAX_SEMIDS 64
#define MAX_NOTIFYIDS 64

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE
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
/// @endcond

////////////////////////////////////////////////////////////////////////////////
// Common Mutex Classes
////////////////////////////////////////////////////////////////////////////////

// class EMutexPrivate;
// class EMutexPublic;
// class EMutexLock;

/// @brief Contains the data associated with a public or private mutex.
/// @details This class is intended to be accessed via a derived object
///          (EMutexPrivate or EMutexPublic).  It is used by EMutexLock.
///          If EpcTools is compiled with NATIVE_IPC, then the underlying
///          mutex is a pthread mutex, otherwise the mutex is implemented
///          as a modified spin lock.
class EMutexData
{
   friend class EMutexPrivate;
   friend class EMutexPublic;
   friend class EMutexLock;

public:
   /// @brief Class destructor.
   ~EMutexData()
   {
   }

   /// @brief Initializes the mutex data.
   /// @param shared indicates that this mutex is to be shared across
   ///   processes.  This flag is only used if NATIVE_IPC is enabled.
   Void init(Bool shared);
   /// @brief Destroyes the mutex data.
   Void destroy();

   /// @brief Retrieves object initialization status.
   /// @return True indicates the object is initialized, otherwise False.
   Bool initialized()
   {
      return m_initialized;
   }

   /// @brief Retrieves the underlying mutex object.
#if defined(NATIVE_IPC)
   /// @return The underlying pthread_mutex_t handle.
   pthread_mutex_t &mutex()
   {
      return m_mutex;
   }
#else
   /// @return The underlying Long mutex variable.
   Long &mutex()
   {
      return m_lock;
   }
#endif

protected:
   /// @cond DOXYGEN_EXCLUDED
   EMutexData()
   {
      m_initialized = False;
   }
   Bool enter(Bool wait = True);
   Void leave();
   /// @endcond

private:
   Bool m_initialized;
#if defined(NATIVE_IPC)
   pthread_mutex_t m_mutex;
#else
   Long m_lock;
#endif
};

/// @brief Acquires and holds a lock on the specified mutex.
class EMutexLock
{
public:
   /// @brief Class constructor.
   /// @param mtx the mutex to lock.
   /// @param acquire if True, the associated mutex is locked in the constructor.
   EMutexLock(EMutexData &mtx, Bool acquire = True)
      : m_acquire(acquire), m_mtx(mtx)
   {
      if (m_acquire)
         m_mtx.enter();
   }
   /// @brief Class destructor.  Unlocks the mutex if locked.
   ~EMutexLock()
   {
      if (m_acquire)
         m_mtx.leave();
   }

   /// @brief Manually acquires a lock on the mutex.
   /// @param wait directs the method to wait for the mutex to be locked.
   /// @return True indicates that the lock was acquired, otherwise False.
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

/// @brief A private mutex (the mutex data is allocated from either the heap or stack).
class EMutexPrivate : public EMutexData
{
public:
   /// @brief Class constructor.
   /// @param bInit directs the constructor to initialize the mutex.
   EMutexPrivate(Bool bInit = True)
       : EMutexData()
   {
      if (bInit)
         init();
   }
   /// @brief Class destructor.
   ~EMutexPrivate()
   {
      destroy();
   }

   /// @brief The overloaded new operator.
   Void *operator new(size_t, Void *where) { return where; }

   Void init() { EMutexData::init(False); }
};

////////////////////////////////////////////////////////////////////////////////
// Public Mutex Classes
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDED
class EMutexDataPublic : public EMutexData
{
   friend class ESynchObjects;

public:
   ~EMutexDataPublic() {}

   Int &mutexId() { return m_mutexId; }

protected:
   EMutexDataPublic() {}
   Int &nextIndex() { return m_nextIndex; }

private:
   Int m_nextIndex;
   Int m_mutexId;
};
/// @endcond

/// @brief A public mutex (the mutex data is located in shared memory).
class EMutexPublic
{
public:
   /// @brief Default constructor.
   /// @param bInit if True, the init method will be called from the constructor.
   EMutexPublic(Bool bInit = True)
   {
      m_mutexid = 0;
      if (bInit)
         init();
   }
   /// @brief Class destructor.
   ~EMutexPublic()
   {
      destroy();
   }

   /// @brief Allocates and initializes the mutex object.
   Void init();
   /// @brief Destroys/releases the mutex object.
   Void destroy();

   /// @brief Associates this public mutex object with the mutex ID located in shared memory.
   /// @param mutexid the identifier of the mutex.
   Void attach(Int mutexid);
   /// @brief Detaches from the public mutex object.
   Void detach();

   /// @brief Retrieves the mutex ID associated with this public mutex.
   /// @return 
   Int mutexId() { return m_mutexid; }

   /// @brief Reference operator overload.
   /// @return Retrieves the mutex data associated with this public mutex.
   operator EMutexDataPublic &();

private:
   Int m_mutexid;
};

////////////////////////////////////////////////////////////////////////////////
// Common Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

/// @brief Contains the data associated with a public or private semaphore.
class ESemaphoreData
{
public:
   /// @brief Default constructor.
   ESemaphoreData()
      : m_initialized(False),
        m_shared(False),
        m_initCount(0),
        m_currCount(0)
   {
   }
   /// @brief Class constructor.
   /// @param initcnt 
   /// @param shared 
   ESemaphoreData(Long initcnt, Bool shared)
      : m_initialized(False),
        m_shared(shared),
        m_initCount(initcnt),
        m_currCount(0)
   {
   }
   /// @brief Class destructor.
   ~ESemaphoreData()
   {
      destroy();
   }

   /// @brief Initializes the semaphore data.
   Void init();
   /// @brief Destroys the semaphore data.
   Void destroy();

   /// @brief Decrements the semaphore.
   /// @param wait if True, this method will block until the semaphore can be
   ///   decremented (when the current value is less than or equal to zeor).  
   /// @return True if the semaphore was successfully decremented, otherwise False.
   Bool Decrement(Bool wait = True);
   /// @brief Increments teh semaphore.
   /// @return True indicates that the semaphore was successfully incremented, otherwise False.
   Bool Increment();

   /// @brief Retrieves the initialization status.
   /// @return True indicates the semahpore data has been initialized, otherwise False.
   Bool initialized() { return m_initialized; }
   /// @brief Retrieves a reference indicating if this is a shared semaphore (public).
   /// @return Reference to shared indicator.
   Bool &shared() { return m_shared; }
   /// @brief Retrieves the initial count of the semaphore.
   /// @return Reference to the initial count value.
   Long &initialCount() { return m_initCount; }
   /// @brief Retrieves the current semaphore count.
   /// @return the current semaphore count.
   Long currCount() { return m_currCount; }

private:
   Bool m_initialized;
   Bool m_shared;
   Long m_initCount;
   Long m_currCount;
   sem_t m_sem;
};

/// @brief Contains the base functionality for a semaphore.
class ESemaphoreBase
{
public:
   /// @brief Default constructor.
   ESemaphoreBase() {}
   /// @brief Class desctructor.
   ~ESemaphoreBase() {}

   /// @brief Initializes the semaphore data.  This method must be overloaded in a derived class.
   /// @param initcnt the initial count for the semaphore.
   virtual Void init(Long initcnt) = 0;
   /// @brief Destroys/releases the semaphore data.  This method must be overloaded in a derived class.
   virtual Void destroy() = 0;

   /// @brief Decrements the semaphore value.
   /// @param wait indicates if the this method will block until the semaphore value is greater than zero.
   /// @return True indicates that the semaphore value was successfully decremented, otherwise False.
   Bool Decrement(Bool wait = True) { return getData().Decrement(wait); }
   /// @brief Increments the semaphore value.
   /// @return True indicates that the semaphore value was successfully decremented, otherwise False.
   Bool Increment() { return getData().Increment(); }

   /// @brief Indicates the initialization status for this object.
   /// @return True indicates the object is initialized, otherwise False.
   Bool initialized() { return getData().initialized(); }
   /// @brief Indicates if this object is to be shared between processes.
   /// @return True semaphore is to be shared, otherwise False.
   Bool &shared() { return getData().shared(); }
   /// @brief Retrieves the initial semaphore value.
   /// @return the initial semaphore value.
   Long &initialCount() { return getData().initialCount(); }
   /// @brief Retrieves the current semaphore value.
   /// @return the current semaphore value.
   Long currCount() { return getData().currCount(); }

   /// @brief Retrieves a reference to the underlying semaphore data.
   /// @return a reference to the underlying semaphore data.
   operator ESemaphoreData&() { return getData(); }

protected:
   /// @cond DOXYGEN_EXCLUDE
   virtual ESemaphoreData &getData() = 0;
   /// @endcond
};

////////////////////////////////////////////////////////////////////////////////
// Private Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

/// @brief Represents a private semaphore, the semaphore data is allocated from
///   either the stack or heap.
class ESemaphorePrivate : public ESemaphoreBase
{
public:
   /// @brief Class constructor.
   /// @param initcnt the initial count for the semaphore.
   /// @param bInit directs the constructor to initialize the semaphore.
   ESemaphorePrivate(Long initcnt=0, Bool bInit = True)
      : m_data(initcnt, False)
   {
      if (bInit)
         init(initcnt);
   }
   /// @brief Class destructor.
   ~ESemaphorePrivate()
   {
      destroy();
   }

   /// @brief Initializes the data associated with the semaphore.
   /// @param initcnt the initial count for the semaphore.
   Void init(Long initcnt) { m_data.initialCount() = initcnt; m_data.init(); }
   /// @brief Destroys the data associated with the semaphore.
   Void destroy() { m_data.destroy(); }

protected:
   /// @cond DOXYGEN_EXCLUDE
   ESemaphoreData &getData() { return m_data; }
   /// @endcond

private:
   ESemaphoreData m_data;
};

////////////////////////////////////////////////////////////////////////////////
// Public Semaphore Classes
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDED
class ESemaphoreDataPublic : public ESemaphoreData
{
   friend class ESemaphorePublic;
   friend class ESynchObjects;
public:
   ESemaphoreDataPublic(Long initcnt=0)
      : ESemaphoreData(initcnt, True)
   {
   }
   ~ESemaphoreDataPublic()
   {
   }

   Int &semIndex() { return m_semIndex; }

protected:
   Int &nextIndex() { return m_nextIndex; }

private:
   Int m_nextIndex;
   Int m_semIndex;
};
/// @endcond

/// @brief Represents a public semaphore, the semaphore data is located in
///   shared memory.
class ESemaphorePublic : public ESemaphoreBase
{
   friend class ESynchObjects;
public:
   /// @brief Default constructor.
   ESemaphorePublic()
      : m_semid(0)
   {
   }
   /// @brief Class constructor.
   /// @param initcnt the initial count for the semaphore.
   /// @param bInit directs the constructor to initialize the semaphore.
   ESemaphorePublic(Long initcnt, Bool bInit = True)
      : m_semid(0)
   {
      if (bInit)
         init(initcnt);
   }
   /// @brief Class destructor.
   ~ESemaphorePublic()
   {
      destroy();
   }

   /// @brief Initializes the data associated with the semaphore.
   /// @param initialCount the initial count for the semaphore.
   Void init(Long initialCount);
   /// @brief Destroys the data associated with the semaphore.
   Void destroy();

   /// @brief Retrieves the semaphore data index.
   /// @return reference to the semaphore data index.
   Int &semIndex();

   /// @brief Attaches this object to the semaphore data associated with the semaphore ID.
   /// @param semid the semaphore ID.
   Void attach(Int semid);
   /// @brief Detaches from the semaphore data.
   Void detach();

protected:
   /// @cond DOXYGEN_EXCLUDE
   Int &nextIndex();
   ESemaphoreData &getData();
   /// @endcond

private:
   Int m_semid;
};

////////////////////////////////////////////////////////////////////////////////
// Private Read/Write Lock Classes
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED2(ERWLockError_LockAttrInitFailed);
DECLARE_ERROR_ADVANCED2(ERWLockError_LockInitFailed);

class ERDLock;
class EWRLock;

/// @brief Encapsulates a read-write lock object.
class ERWLock
{
   friend ERDLock;
   friend EWRLock;

public:
   /// @brief Default constructor.
   ERWLock();
   /// @brief Class destructor.
   ~ERWLock();

protected:
   /// @cond DOXYGEN_EXCLUDE
   enum ReadWrite
   {
      Read,
      Write
   };

   Bool enter( ReadWrite rw, Bool wait );
   Void leave();
   /// @endcond

private:
   pthread_rwlock_t m_rwlock;
};

/// @brief Request a read lock for the specified read-write lock object.
class ERDLock
{
public:
   /// @brief Class constructor.
   /// @param rwlock the read-write lock object to obtain the read lock for.
   /// @param acq directs the constructor to acquire the lock.
   ERDLock( ERWLock & rwlock, Bool acq = true )
      : m_rwlock( rwlock ),
        m_locked( false )
   {
      if (acq)
         acquire();
   }
   /// @brief Class destructor.
   ~ERDLock()
   {
      if (m_locked)
         m_rwlock.leave();
   }

   /// @brief Acquires the lock.
   /// @param wait if True, this method will wait for the lock to be obtained.
   /// @return True indicates that the lock was successfully acquired, otherwise False.
   Bool acquire( Bool wait = true )
   {
      return m_locked = m_rwlock.enter( ERWLock::Read, wait );
   }
   /// @brief Retrieves the lock status.
   /// @return True indicates that the lock has been acquired, otherwise False.
   Bool isLocked() { return m_locked; }

private:
   ERDLock();

   ERWLock &m_rwlock;
   Bool m_locked;
};

/// @brief Request a write lock for the specified read-write lock object.
class EWRLock
{
public:
   /// @brief Class constructor.
   /// @param rwlock the read-write lock object to obtain the write lock for.
   /// @param acq directs the constructor to acquire the lock.
   EWRLock( ERWLock & rwlock, Bool acq = true )
      : m_rwlock( rwlock ),
        m_locked( false )
   {
      if (acq)
         acquire();
   }
   /// @brief Class destructor.
   ~EWRLock()
   {
      if (m_locked)
         m_rwlock.leave();
   }

   /// @brief Acquires the lock.
   /// @param wait if True, this method will wait for the lock to be obtained.
   /// @return True indicates that the lock was successfully acquired, otherwise False.
   Bool acquire( Bool wait = true )
   {
      return m_locked = m_rwlock.enter( ERWLock::Write, wait );
   }
   /// @brief Retrieves the lock status.
   /// @return True indicates that the lock has been acquired, otherwise False.
   Bool isLocked() { return m_locked; }

private:
   EWRLock();

   ERWLock &m_rwlock;
   Bool m_locked;
};

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/// @brief An object that can be waited on to be set in another thread.
class EEvent
{
public:
   /// @brief Class constructor.
   /// @param state the initial state of the event.
   EEvent( bool state = false );
   /// @brief Class destructor.
   ~EEvent();

   /// @brief Resets the event state.
   Void reset();
   /// @brief Sets the event.
   Void set();
   /// @brief Waits for the event to be set.
   /// @param ms if -1, this function waits indefinitely, otherwise waits the
   ///   specified number of milli-seonds for the event to be set.
   /// @return True if the event has been sent, otherwise False.
   Bool wait( int ms = -1 );
   /// @brief Determines if the event has been set.
   /// @return True indicates that the event is set, otherwise False.
   Bool isSet() { return wait(0); }

private:
   void closepipe();

   int m_pipefd[2];
   char m_buf[1];
};

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#endif // #define __esynch_h_included
